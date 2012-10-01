require 'uri'
require 'timeout'

DEBUGGER_STEP_TYPE = ['STEP','STOVER','STRET','SUSP']
DEBUGGER_STEP_COMMENT = ['Stepped into','Stepped over','Stepped return','Suspended']

DEBUGGER_LOG_LEVEL_DEBUG  = 0
DEBUGGER_LOG_LEVEL_INFO  = 1
DEBUGGER_LOG_LEVEL_WARN  = 2
DEBUGGER_LOG_LEVEL_ERROR = 3

def debugger_log(level, msg)
  if (level >= DEBUGGER_LOG_LEVEL_WARN)
    puts "[Debugger] #{msg}"
  end
end

def log_command(cmd)
  debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, "Received command: #{cmd}")
end

def debug_read_cmd(io,wait)
  begin
    if wait
      cmd = io.readpartial(4096)
      $_cmd << cmd if cmd !~ /^\s*$/
    else
      cmd = io.read_nonblock(4096)
      $_cmd << cmd if cmd !~ /^\s*$/
    end
    # $_s.write("get data from front end" + $_cmd.to_s + "\n")
  rescue
    # puts $!.inspect
  end
end

def execute_cmd(cmd, advanced)
  #$_s.write("execute_cmd start\n")
  cmd = URI.unescape(cmd.gsub(/\+/,' ')) if advanced
  debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, "Executing: #{cmd.inspect}")
  result = ""
  error = '0';
  begin
    result = eval(cmd.to_s, $_binding).inspect
  rescue Exception => exc
    error = '1';
    result = "#{$!}".inspect
  end
  
  cmd = URI.escape(cmd.sub(/[\n\r]+$/, ''), Regexp.new("[^#{URI::PATTERN::UNRESERVED}]")) if advanced
  $_s.write("EV" + (advanced ? "L:#{error}:#{cmd}:" : ':'+(error.to_i != 0 ? 'ERROR: ':'')) + result + "\n")
  #$_s.write("execute_cmd end\n")
end

def get_variables(scope)  
  #$_s.write("get_variables start\n")

  if (scope =~ /^GVARS/)
   cmd = "global_variables"
   prefix = ""
   vartype = "G"
  elsif (scope =~ /^LVARS/)
   cmd = "local_variables"
   prefix = ""
   vartype = "L"
  elsif (scope =~ /^CVARS/)
   if $_classname =~ /^\s*$/
     return
   end
   cmd = "class_variables"
   prefix = "#{$_classname}."
   vartype = "C"
  elsif (scope =~ /^IVARS/)
   if ($_classname =~ /^\s*$/) || ($_methodname =~ /^\s*$/)
     return
   end
   cmd = "instance_variables"
   prefix = "self."
   vartype = "I"
  end
  begin
    vars = eval(prefix + cmd, $_binding)
    $_s.write("VSTART:#{vartype}\n")
    vars.each do |v|
      if v !~ /^\$(=|KCODE|-K)$/
        begin
          result = eval(v.to_s, $_binding).inspect
        rescue Exception => exc
          $_s.write("get var exception\n")
          result = "#{$!}".inspect
        end
        $_s.write("V:#{vartype}:#{v}:#{result}\n")
      end
    end
    $_s.write("VEND:#{vartype}\n")
  rescue
  end
end

def debug_handle_cmd(inline)
  #$_s.write("start of debug_handle_cmd wait=" + inline.to_s + "\n")

  cmd = $_cmd.match(/^([^\n\r]*)([\n\r]+|$)/)[0]
  processed = false
  wait = inline

  if cmd != ""
    if cmd =~/^CONNECTED/
      log_command(cmd)
      debugger_log(DEBUGGER_LOG_LEVEL_INFO, "Connected to debugger")
      processed = true
    elsif cmd =~/^(BP|RM):/
      log_command(cmd)
      ary = cmd.split(":")
      bp = ary[1].gsub(/\|/,':') + ':' + ary[2].chomp
      if (cmd =~/^RM:/)
        $_breakpoint.delete(bp)
        debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, "Breakpoint removed: #{bp}")
      else
        $_breakpoint.store(bp,1)
        debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, "Breakpoint added: #{bp}")
      end
      processed = true
    elsif cmd =~ /^RMALL/
      log_command(cmd)
      $_breakpoint.clear
      debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, "All breakpoints removed")
      processed = true
    elsif cmd =~ /^ENABLE/
      log_command(cmd)
      $_breakpoints_enabled = true
      debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, "Breakpoints enabled")
      processed = true
    elsif cmd =~ /^DISABLE/
      log_command(cmd)
      $_breakpoints_enabled = false
      debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, "Breakpoints disabled")
      processed = true
    elsif inline && (cmd =~ /^STEPOVER/)
      $_s.write("STEPOVER start\n")
      log_command(cmd)
      $_step = 2
      $_step_level = $_call_stack
      $_resumed = true
      wait = false
      debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, "Step over")
      processed = true
    elsif inline && (cmd =~ /^STEPRET/)
      log_command(cmd)
      if $_call_stack < 1
        $_step = 0
        comment = ' (continue)'
      else
        $_step = 3
        $_step_level = $_call_stack-1;
        comment = ''
      end
      $_resumed = true
      wait = false
      debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, "Step return" + comment)
      processed = true
    elsif inline && (cmd =~ /^STEP/)
      log_command(cmd)
      $_step = 1
      $_step_level = -1
      $_resumed = true
      wait = false
      debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, "Step into")
      processed = true
    elsif inline && (cmd =~ /^CONT/)
      log_command(cmd)
      wait = false
      $_step = 0
      $_resumed = true
      debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, "Resuming")
      processed = true
    elsif cmd =~ /^SUSP/
      log_command(cmd)
      $_step = 4
      $_step_level = -1
      wait = true
      debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, "Suspend")
      processed = true
    elsif cmd =~ /^KILL/
      log_command(cmd)
      debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, "Terminating...")
      processed = true
      System.exit
    elsif inline && (cmd =~ /^EVL?:/)
      log_command(cmd)
      processed = true
      debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, "Calc evaluation...")
      execute_cmd cmd.sub(/^EVL?:/,""), (cmd =~ /^EVL:/ ? true : false)
    elsif inline && (cmd =~ /^[GLCI]VARS/)
      log_command(cmd)
      debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, "Get variables...")
      get_variables cmd
      processed = true
    elsif inline
      log_command(cmd)
      debugger_log(DEBUGGER_LOG_LEVEL_WARN, "Unknown command")
      processed = true
    end
  end

  if processed
    $_cmd = $_cmd.sub(/^([^\n\r]*)([\n\r]+(.*)|)$/, "\\3")
    $_wait = wait if inline
  end

  #$_s.write("end of debug_handle_cmd wait=" + $_wait.to_s + "cmd=" + cmd + "\n")

  processed
end

$_tracefunc = lambda{|event, file, line, id, bind, classname|
  return if eval('Thread.current!=Thread.main', bind)
  $_binding = bind;
  $_classname = classname;
  $_methodname = id;
  file = file.to_s.gsub('\\', '/')

  #$_s.write('[Debugger][1] file = ' + file.to_s + ' line = ' + line.to_s + "\n")

  if (file[0, $_app_path.length] == $_app_path) || (!(file.index("./").nil?)) || (!(file.index("framework").nil?)) || (!(file.index("extensions").nil?))

    #$_s.write('[Debugger][2] file = ' + file.to_s + ' line = ' + line.to_s + "\n")

    if event =~ /^line/
      unhandled = true
      step_stop = ($_step > 0) && (($_step_level < 0) || ($_call_stack <= $_step_level))

      #$_s.write('[Debugger][3] bp list = ' +  $_breakpoint.to_s + "\n")

      if (step_stop || ($_breakpoints_enabled && (!($_breakpoint.empty?))))
        filename = ""
         
        if !(file.index("./").nil?)
          filename = "/" + file[file.index("./") + 2, file.length]
        elsif !(file.index("framework").nil?)
          filename = file[file.index("framework"), file.length]
        elsif !(file.index("extensions").nil?)
          filename = file[file.index("extensions"), file.length]
        else
          filename = file[$_app_path.length, file.length-$_app_path.length]
        end

        ln = line.to_i.to_s

        #$_s.write('[Debugger][3] $_breakpoints_enabled = ' +  $_breakpoints_enabled.to_s + "\n")
        #$_s.write('[Debugger][3] filename = ' +  filename.to_s + "\n")
        #$_s.write('[Debugger][3] line = ' +  ln.to_s + "\n")
        #$_s.write('[Debugger][3] flag = ' +  ($_breakpoint.has_key?(filename + ':' + ln)).to_s + "\n")

        if step_stop || ($_breakpoints_enabled && ($_breakpoint.has_key?(filename + ':' + ln)))
          #$_s.write('[Debugger][3] stop on bp ' +  filename + ':' + ln + "\n")

          fn = filename.gsub(/:/, '|')
          cl = classname.to_s.gsub(/:/,'#')
          $_s.write((step_stop ? DEBUGGER_STEP_TYPE[$_step-1] : "BP") + ":#{fn}:#{ln}:#{cl}:#{id}\n")
          debugger_log(DEBUGGER_LOG_LEVEL_DEBUG, (step_stop ? DEBUGGER_STEP_COMMENT[$_step-1] : "Breakpoint") + " in #{fn} at #{ln}")
          $_step = 0
          $_step_level = -1

          #$_s.write("start waiting\n")

          app_type = ENV["APP_TYPE"]
          $_wait = true
          while $_wait
            while debug_handle_cmd(true) do end

            if app_type.eql? "rhodes"
              if System::get_property('main_window_closed')
                 $_s.write("QUIT\n") if !($_s.nil?)
                 $_wait = false
              end
            end

            sleep(0.1) if $_wait
          end

          #$_s.write("end waiting\n")

          unhandled = false
        end
      end

      if unhandled
        debug_handle_cmd(true)
      end

    elsif event =~ /^call/
      $_call_stack += 1
    elsif event =~ /^return/
      $_call_stack -= 1
    end
  end

  if $_resumed
    $_resumed = false
    $_s.write("RESUMED\n")
  end
}


$_s = nil

begin
  debugger_log(DEBUGGER_LOG_LEVEL_INFO, "Opening connection")
  debug_host_env = ENV['RHOHOST']
  debug_port_env = ENV['rho_debug_port']
  debug_path_env = ENV['ROOT_PATH']

  debug_host = ((debug_host_env.nil?) || (debug_host_env == "")) ? '127.0.0.1' : debug_host_env 
  debug_port = ((debug_port_env.nil?) || (debug_port_env == "")) ? 9000 : debug_port_env  

  debugger_log(DEBUGGER_LOG_LEVEL_INFO, "host=" + debug_host_env.to_s)
  debugger_log(DEBUGGER_LOG_LEVEL_INFO, "port=" + debug_port_env.to_s)
  debugger_log(DEBUGGER_LOG_LEVEL_INFO, "path=" + debug_path_env.to_s)

  $_s = timeout(30) { TCPSocket.open(debug_host, debug_port) }

  debugger_log(DEBUGGER_LOG_LEVEL_WARN, "Connected: " + $_s.to_s)
  $_s.write("CONNECT\nHOST=" + debug_host.to_s + "\nPORT=" + debug_port.to_s + "\n")
 
  $_breakpoint = Hash.new
  $_breakpoints_enabled = true
  $_step = 0
  $_step_level = -1
  $_call_stack = 0
  $_resumed = false
  $_cmd = ""
  $_app_path = ""

  $_app_path = ((debug_path_env.nil?) || (debug_path_env == "")) ? "" : debug_path_env  
  $_s.write("DEBUG PATH=" + $_app_path.to_s + "\n")

  at_exit {
    $_s.write("QUIT\n") if !($_s.nil?)
  }

  set_trace_func $_tracefunc

  Thread.new {
    while true
      debug_read_cmd($_s,true)
      while debug_handle_cmd(false) do end
      if ($_cmd !~ /^\s*$/) && (Thread.main.stop?)
        #$_s.write("[manage thread] set wait = true\n")
        $_wait = true
        Thread.main.wakeup
      end
    end
  }

rescue
  debugger_log(DEBUGGER_LOG_LEVEL_ERROR, "Unable to open connection to debugger: " + $!.inspect)
  $_s = nil
end
