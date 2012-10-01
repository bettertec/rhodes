#------------------------------------------------------------------------
# (The MIT License)
# 
# Copyright (c) 2008-2011 Rhomobile, Inc.
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# http://rhomobile.com
#------------------------------------------------------------------------

#common function tools to work with android devices
#
# uses following globals
# $adb

#USE_TRACES = Rake.application.options.trace

module AndroidTools

def fill_api_levels(sdkpath)
    $api_levels = Hash.new
    $market_versions = Hash.new
    max_apilevel = 0
    max_platform = nil 
    
    Dir.glob(File.join(sdkpath, "platforms", "*")).each do |platform|
      props = File.join(platform, "source.properties")
      unless File.file? props
        puts "+++ WARNING! No source.properties found in #{platform}"
        next
      end

      apilevel = 0
      marketversion = nil
      File.open(props, "r") do |f|
        while line = f.gets
          apilevel = $1.to_i if line =~ /^\s*AndroidVersion\.ApiLevel\s*=\s*([0-9]+)\s*$/
          marketversion = $1 if line =~ /^\s*Platform\.Version\s*=\s*([^\s]*)\s*$/
        end
      end

      puts "+++ API LEVEL of #{platform}: #{apilevel}" if USE_TRACES

      if apilevel != 0
        $api_levels[marketversion] = apilevel
        $market_versions[apilevel] = marketversion
        if apilevel > max_apilevel
          max_apilevel = apilevel
          max_platform = File.basename platform
        end
      end
    end
    
    max_platform
end
module_function :fill_api_levels

def get_installed_market_versions
    $api_levels.keys
end
module_function :get_installed_market_versions

def get_installed_api_levels
    $market_versions.keys
end
module_function :get_installed_api_levels

def get_market_version(apilevel)
    $market_versions[apilevel]
end
module_function :get_market_version

def get_api_level(marketversion)
    $api_levels[marketversion]
end
module_function :get_api_level

def get_addon_classpath(libnames, apilevel = nil)

    if USE_TRACES
      puts "Looking for #{libnames.inspect}"
      puts "Looking for apilevel #{apilevel}" if apilevel
    end

    libpatterns = []
    found_classpath = nil
    found_apilevel = nil
    libnames.each do |name|
      libpatterns << Regexp.new("^(#{name})=(.+);.*$")
    end

    Dir.glob(File.join($androidsdkpath, 'add-ons', '*')).each do |dir|
        next unless File.directory? dir
    
        libs = {}
        cur_apilevel = nil
        classpath = nil
        props = File.join(dir, 'manifest.ini')
        unless File.file? props
            puts "+++ WARNING: no manifest.ini found in #{dir}"
            next
        end
        
        libs = {}
        File.open(props, 'r') do |f|
          while line = f.gets
            if line =~ /^api=([0-9]+)$/
              cur_apilevel = $1.to_i

              puts "API level of #{dir}: #{cur_apilevel}" if USE_TRACES

              break if apilevel and apilevel != cur_apilevel
              break if found_apilevel and found_apilevel > cur_apilevel
            end
            libpatterns.each do |pat|
              if(pat =~ line)
                libs[$1] = $2
              end
            end
          end
        end
        
        next if apilevel and apilevel != cur_apilevel
        next if found_apilevel and cur_apilevel < found_apilevel

        libnames.each do |name|
          if libs[name]
            if classpath
              classpath += $path_separator
            else
              classpath = ''
            end
            classpath += File.join(dir,'libs',libs[name])
          else
            classpath = nil
            break
          end
        end
        
        next unless classpath
        
        found_apilevel = cur_apilevel
        found_classpath = classpath
        
        puts "classpath: #{found_classpath.inspect}, API level: #{found_apilevel}" if USE_TRACES
        
    end

    unless found_classpath
      msg = "No Android SDK add-on found for libraries: #{libnames.inspect}"
      msg += "; API level: #{apilevel}" if apilevel
      raise msg
    end
    
    if USE_TRACES
      puts "Add-on libraries: #{libnames.inspect}"
      puts "Add-on classpath: #{found_classpath}"
      puts "Add-on API level: #{found_apilevel}"
    end

    found_classpath
end
module_function :get_addon_classpath

def get_app_log (appname, device, silent = false)
  pkgname = "com.#{$vendor}." + appname.downcase.gsub(/[^A-Za-z_0-9]/, '')
  path = File.join('/data/data', pkgname, 'rhodata', 'RhoLog.txt')
  Jake.run($adb, [device ? '-d' : '-e', 'pull', path, $app_path]) or return false
  puts "RhoLog.txt stored to " + $app_path
  return true
end
module_function :get_app_log  

def is_emulator_running
  system("\"#{$adb}\" start-server")
  `"#{$adb}" devices`.split("\n")[1..-1].each do |line|
    return true if line =~ /^emulator/
  end
  return false
end
module_function :is_emulator_running

def is_device_running
  system("\"#{$adb}\" start-server")
  `"#{$adb}" devices`.split("\n")[1..-1].each do |line|
    return true if line !~ /^emulator/
  end
  return false
end
module_function :is_device_running

def  run_emulator(options = {})
  system("\"#{$adb}\" start-server")

  rm_f $applog_path if !$applog_path.nil?
  logcat_process()
  
  unless is_emulator_running
    puts "Need to start emulator" if USE_TRACES

    if $appavdname
      $avdname = $appavdname
    else
      $avdname = "rhoAndroid" + $emuversion.gsub(/[^0-9]/, "")
      $avdname += "google" if $use_google_addon_api
      $avdname += "motosol" if $use_motosol_api
    end
    
    targetid = $androidtargets[get_api_level($emuversion)]
    
    unless File.directory?( File.join(ENV['HOME'], ".android", "avd", "#{$avdname}.avd" ) )
      if USE_TRACES
        puts "AVD name: #{$avdname}, emulator version: #{$emuversion}, target id: #{targetid}"
      end
      raise "Unable to create AVD image. No appropriate target API for SDK version: #{$emuversion}" unless targetid
      createavd = "\"#{$androidbin}\" create avd --name #{$avdname} --target #{targetid} --sdcard 128M"
      puts "Creating AVD image: #{$avdname}"
      IO.popen(createavd, 'r+') do |io|
        io.puts "\n"
        while line = io.gets
          puts line
        end
      end
      
    else
      raise "Unable to run Android emulator. No appropriate target API for SDK version: #{$emuversion}" unless targetid
    end

    # Start the emulator, check on it every 5 seconds until it's running
    cmd = "\"#{$emulator}\" -cpu-delay 0"
    cmd << " -no-window" if options[:hidden]
    cmd << " -avd #{$avdname}"
    Thread.new { system(cmd) }

    puts "Waiting for emulator..."
    res = 'error'        
    while res =~ /error/ do
      sleep 5
      res = Jake.run $adb, ['-e', 'wait-for-device']
      puts res
    end

    puts "Waiting up to 600 seconds for emulator..."
    startedWaiting = Time.now
    adbRestarts = 1
    while (Time.now - startedWaiting < 600 )
        sleep 5
        now = Time.now
        started = false
        booted = true
        Jake.run2 $adb, ["-e", "shell", "ps"], :system => false, :hideerrors => false do |line|
            #puts line
            booted = false if line =~ /bootanimation/
            started = true if line =~ /android\.process\.acore/
            true
        end
        #puts "started: #{started}, booted: #{booted}"
        unless started and booted
            printf("%.2fs: ",(now - startedWaiting))
            if (now - startedWaiting) > (180 * adbRestarts)
              # Restart the adb server every 60 seconds to prevent eternal waiting
              puts "Appears hung, restarting adb server"
              restart_adb
              Jake.run($adb, ['start-server'], nil, true)
              adbRestarts += 1

              rm_f $applog_path if !$applog_path.nil?
              logcat_process()
            else
              puts "Still waiting..."
            end
        else
            puts "Success"
            puts "Device is ready after " + (Time.now - startedWaiting).to_s + " seconds"
            break
        end
    end
    raise "Emulator still isn't up and running, giving up" unless is_emulator_running
  end

  puts "Emulator is up and running"
  $stdout.flush
end
module_function :run_emulator

def run_application (device_flag, pkgname)
  puts "Starting application.."
  args = []
  args << device_flag
  args << "shell"
  args << "am"
  args << "start"
  args << "-a"
  args << "android.intent.action.MAIN"
  args << "-n"
  args << pkgname + "/#{JAVA_PACKAGE_NAME}.RhodesActivity"
  Jake.run($adb, args)
end
module_function :run_application

def application_running(device_flag, pkgname)
  pkg = pkgname.gsub(/\./, '\.')
  system("\"#{$adb}\" start-server")
  `"#{$adb}" #{device_flag} shell ps`.split.each do |line|
    return true if line =~ /#{pkg}/
  end
  false
end
module_function :application_running

def load_app_and_run(device_flag, apkfile, pkgname)
  device = 'started emulator'
  if device_flag == '-d'
    device = 'connected device'
  end

  puts "Loading package..."

  argv = [$adb, device_flag, "install", "-r", apkfile]
  cmd = ""
  argv.each { |arg| cmd << "#{arg} "}
  argv = cmd if RUBY_VERSION =~ /^1\.8/
  #cmd = "#{$adb} #{device_flag} install -r #{apkfile}"
  #puts "CMD: #{cmd}" 

  count = 0
  done = false
  child = nil
  while count < 20
    theoutput = ""
    begin
      status = Timeout::timeout(30) do
        puts "CMD: #{cmd}"
        IO.popen(argv) do |pipe|
          child = pipe.pid
          while line = pipe.gets
            theoutput << line
            puts "RET: #{line}"
          end
        end
      end
    rescue Timeout::Error
      Process.kill 9, child if child
      if theoutput == ""
        puts "Timeout reached while empty output: killing adb server and retrying..."
        `#{$adb} kill-server`
        count += 1
        sleep 1
        next
      else
        puts "Timeout reached: try to run application"
        done = true
        break
      end
    end

    if theoutput.to_s.match(/Success/)
      done = true
      break
    end
    
    if theoutput.to_s.match(/INSTALL_PARSE_FAILED_INCONSISTENT_CERTIFICATES/)
      raise "Inconsistent sertificates: please, uninstall application signed with another sertificate from #{device} first"
    end
    if theoutput.to_s.match(/INSTALL_FAILED_MISSING_SHARED_LIBRARY/)
      raise "Missing shared library: application is not compatible with #{device} due to lack of required libraries"
    end
    
    puts "Failed to load (possibly because emulator/device is still offline) - retrying"
    $stdout.flush
    sleep 1
    count += 1
  end

  run_application(device_flag, pkgname) if done
end
module_function :load_app_and_run

def restart_adb
  puts 'Killing adb server'
  system("#{$adb} kill-server")
  #if RUBY_PLATFORM =~ /(win|w)32$/
  #  # Windows
  #  system ('taskkill /F /IM adb.exe')
  #else
  #  system ('killall -9 adb')
  #end
  sleep 3
  puts 'Starting adb server again'
  system("#{$adb} start-server")
  sleep 3
end
module_function :restart_adb

def kill_adb_and_emulator
  if RUBY_PLATFORM =~ /windows|cygwin|mingw/
    # Windows
    `taskkill /F /IM adb.exe`
    `taskkill /F /IM emulator-arm.exe`
    `taskkill /F /IM emulator.exe`
  else
    `killall -9 adb`
    `killall -9 emulator-arm`
    `killall -9 emulator`
  end
end
module_function :kill_adb_and_emulator

def logcat(device_flag = '-e', log_path = $applog_path)
  if !log_path.nil?
    rm_rf log_path if File.exist?(log_path)
    Thread.new { Jake.run($adb, [device_flag, 'logcat', '>>', log_path], nil, true) }
  end
end
module_function :logcat

def logcat_process(device_flag = '-e', log_path = $applog_path)
  if !log_path.nil?
    Thread.new { system("\"#{$adb}\" #{device_flag} logcat >> \"#{log_path}\" ") }  
  end
end
module_function :logcat_process

def logclear(device_flag = '-e')
  return if(device_flag == '-e' and !is_emulator_running)
  Jake.run($adb, [device_flag, 'logcat', '-c'], nil, true) 
end
module_function :logclear

end
