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

require "fileutils"

def fixWin7SimBug (rhoconfig) 
=begin
	file = rhoconfig
	tmpfile = file + ".tmp"
	no_deviceside_postfix_isFound = 0
	File.open(file, 'r') do |f|
	  File.open(tmpfile, 'w') do |newf|
	    while line = f.gets
	      if line =~ /^\s*no_deviceside_postfix\s*=/
		no_deviceside_postfix_isFound = 1
	      end
	      newf.puts line
	    end
	    if no_deviceside_postfix_isFound == 0
	      newf.puts "no_deviceside_postfix = 1"
	    end
	  end
	end
	FileUtils.move(tmpfile, file)
=end
end


def freplace( fname, pattern, str )
  f = File.open( fname )
  strings = f.read
  f.close

  strings.gsub!( pattern, str )

  f = File.new( fname, "w" )
  f.print strings
  f.close
end

def startmds
  mdshome =  $config["env"]["paths"][$bbver]["mds"]
  args = []
  args << "/c"
  args << "run.bat"

  #Jake.run("cmd.exe",args, mdshome,true)
end 

def stopmds
  mdshome =  $config["env"]["paths"][$bbver]["mds"]
  args = []
  args << "/c"
  args << "shutdown.bat"

  #Jake.run("cmd.exe",args, mdshome, true, true)
end 

def startsim(hidden=false)
  sim = $config["env"]["paths"][$bbver]["sim"]
    
  command =  $bb_sim_path + "/fledge.exe"
  args = []
  args << "/app=Jvm.dll"
  args << "/handheld=" + sim
  args << "/session=" + sim
  args << "/app-param=DisableRegistration"
  args << "/app-param=JvmAlxConfigFile:"+sim+".xml"
  args << "/data-port=0x4d44"
  args << "/data-port=0x4d4e"
  args << "/pin=0x2100000A"
  args << "/no-compact-filesystem"
  args << "/JvmDisableBacklightTimeout"
  args << "/start-hidden" if hidden
  #args << "/keep-lcd-on"
    
  if $bbver !~ /^4\.[012](\..*)?$/
    args << "/sdcard-inserted=true"
    args << "/fs-sdcard=true"
  end
  if $bbver !~ /^4\.[01235](\..*)?$/
    args << "/no-guibacklight"
  end
        
  args << "/app-param=JvmDebugFile:"+Jake.get_absolute_ex($app_config["applog"], $app_path) if $app_config["applog"] && $app_config["applog"].length() > 0

  Jake.run2 command, args, {:directory => $bb_sim_path, :nowait => true}
end

def load_to_sim(is_wait)
  sim = $config["env"]["paths"][$bbver]["sim"]

  cod_path = File.join( $targetdir, $outfilebase+".cod.pending")

  puts "cod_path : #{cod_path}"
  
  command = $bb_sim_path + "/fledgecontroller.exe"
  args = []
  args << "/session="+sim
  args << "/execute=LoadCod(\"#{cod_path}\")"

  while(true)  
    res_line = ""
    Jake.run2( command, args, {:directory => $bb_sim_path} ) do |line| 
      puts "fledgecontroller return:" + line;
      res_line = line
    end  

    if res_line && res_line.index("There is no Fledge instance running with session name")    
      return false unless is_wait
      puts "wait for simulator..."
    else
      break
    end
  end

  
  args = []
  args << "/session="+sim
  args << "/execute=LoadCod(\"updates.force\")"
  Jake.run2 command, args, {:directory => $bb_sim_path, :nowait => true}  
  
  return true
end

def stopsim
  sim = $config["env"]["paths"][$bbver]["sim"]
    
  command = $bb_sim_path + "/fledgecontroller.exe"
  args = []
  args << "/session="+sim
  args << "/execute=Exit(true)"
  Jake.run2 command, args, {:directory => $bb_sim_path, :nowait => true}
end

def manualsign
  jpath = $config["env"]["paths"]["java"]
  java = jpath && jpath.length() > 0 ? File.join(jpath, "java" ) : "java"

  args = []
  args << "-jar"
  args << $bb_jde_path + "/bin/SignatureTool.jar"
  args << "-r"
  args << $targetdir

  puts Jake.run(java,args)
  $stdout.flush

end

def autosign
  jpath = $config["env"]["paths"]["java"]
  java = jpath && jpath.length() > 0 ? File.join(jpath, "java" ) : "java"

  args = []
  args << "-jar"
  args << $bb_jde_path + "/bin/SignatureTool.jar"
  args << "-c"
  args << "-a"
  args << "-p"
  args << $config["build"]["bbsignpwd"]
  args << "-r"
  args << $targetdir

  puts Jake.run(java,args)
  $stdout.flush

end

def create_alx_file(src,trg)
    rbText = ERB.new( IO.read($builddir + "/" + src + "Alx.erb") ).result
	fAlx = File.new($targetdir + "/" + trg + ".alx", "w")
    fAlx.write(rbText)
    fAlx.close()
end


namespace "config" do
  task :set_bb_platform do
    $current_platform = "bb"
    
  end
  
  task :read_bb_version do
    $bbver = $app_config["bbver"].to_s
    unless $app_config[$current_platform] && $app_config[$current_platform]["ignore_bb6_suffix"]  && $app_config[$current_platform]['ignore_bb6_suffix'].to_s == '1'
        $bb6 = true if $bbver.to_f >= 6.0
        
        puts "use bb6 suffix" if $bb6
    end    
  end
      
  task :bb => [:set_bb_platform, "config:common", :read_bb_version] do

    use_sqlite = $app_config[$current_platform] && $app_config[$current_platform]['use_sqlite']  && $app_config[$current_platform]['use_sqlite'].to_s == '1'
    $use_sqlite = $bbver.to_f >= 5.0 && use_sqlite ? true : false
    puts "$use_sqlite : #{$use_sqlite}"
    
    $builddir = $config["build"]["bbpath"] + "/build"
    $bindir = $app_path + "/bin"
    $tmpdir =  $bindir +"/tmp"
    $rhobundledir =  $bindir + "/RhoBundle"
    $srcdir =  $bindir + "/RhoBundle"
    $preverified = $bindir + "/preverified"
    $targetdir = $bindir + "/target/" + $bbver
    $rubyVMdir = $app_path + "/RubyVM"
    $excludelib = ['**/rational.rb','**/dateOrig.rb']
    $excludeextlib = ['rexml/parsers/baseparser.rb', 'rexml/set.rb']
    $compileERB = $app_path + "/build/compileERB.rb"

    $assetfolder = $app_path + "/public-" + "bb-" + $bbver

    $appname = $app_config["name"].nil? ? "rhodesApp" : $app_config["name"]
    $outfilebase = $appname.gsub(/[^A-Za-z_0-9]/, '_')
    $bundleClassName = $outfilebase + '_'  unless $bundleClassName

    $bb_jde_path = $config["env"]["paths"][$bbver]["jde"]
    $bb_lib_path = $bb_jde_path + "/lib/net_rim_api.jar"
    $bb_sim_path = $config["env"]["paths"][$bbver]["sim_path"]    
    $bb_sim_path = $bb_jde_path + "/simulator" unless $bb_sim_path

	jpath = $config["env"]["paths"]["java"]
    $javac = jpath && jpath.length() > 0 ? File.join(jpath, "javac.exe" ) : "javac.exe"
    
    $rhobundleimplib = $bb_lib_path + ";" + $preverified+"/RubyVM.jar"
    $rhodesimplib = $rhobundleimplib + ";"+ $preverified+"/RhoBundle.jar"

    needsclean = true
    if File.exists? "#{$app_path}/last_bbver"
      last_bbver = IO.read("#{$app_path}/last_bbver").strip
      needsclean = false if $bbver == last_bbver
    end

    Rake::Task["clean:bb:all"].invoke if needsclean
    
    mkdir_p $bindir unless File.exists? $bindir
    rm_rf $tmpdir
    mkdir_p $tmpdir unless File.exists? $tmpdir
    mkdir_p  $targetdir if not FileTest.exists?  $targetdir

    File.open("#{$app_path}/last_bbver", "w") {|f| f.write($bbver)}

  end
end

namespace "build" do
  namespace "bb" do
    task :alx => ["config:bb"] do
        create_alx_file('rhodesApp', $outfilebase)
        create_alx_file('RhoBundle', 'RhoBundle')
    end
    
    def runPreverify(args)  
      startdir = Dir.pwd()
      chdir $tmpdir
      puts Jake.run(File.join($bb_jde_path,"bin/preverify"),args)
      chdir startdir
      
      unless $? == 0
        rm_rf $tmpdir
        puts "Error preverifying"
        exit 1
      end
      $stdout.flush
    end
    
#    desc "Build rhoBundle"
    #XXX change to ns build, rhobundle
    task :rhobundle => ["config:checkbb", :rubyvm] do
      jpath = $config["env"]["paths"]["java"]  
      java = jpath && jpath.length()>0 ? File.join( jpath, "java") : "jar"
      jarexe =  jpath && jpath.length()>0 ? File.join( jpath, "jar" ) : "jar"
      
      #common bundle task goes here#
      Rake::Task["build:bundle:xruby"].execute

      rm_rf $srcdir
      rm_rf $preverified + "/RhoBundle.jar" if File.exists? $preverified + "/RhoBundle.jar"
      rm_rf $rhobundledir + "/RhoBundle.jar" if File.exists? $rhobundledir + "/RhoBundle.jar"

      # start build extensions

      extensionsdir = $tmpdir + "/extensions"
      extensionstmpdir = $tmpdir + "/extensions/tmp"

      ENV['RHO_PLATFORM'] = 'bb'
      ENV["TARGET_TEMP_DIR"] = extensionsdir
      
      ENV["RHO_ROOT"] = $startdir
      
      ENV["JAVA_EXE"] = java
      ENV["JAVAC_EXE"] = $javac
      ENV["JDE_HOME"] = $bb_jde_path
      ENV["JAR_EXE"] = jarexe
	
      ENV["RUBYVM_JAR"] = $preverified+"/RubyVM.jar"	

      ENV["BB_SDK_VERSION"] = $bbver

      mkdir_p extensionsdir unless File.directory? extensionsdir
      mkdir_p extensionstmpdir unless File.directory? extensionstmpdir

      extentries = []
      
      puts "$app_config['extensions'] : #{$app_config['extensions']}"
      $app_config["extensions"].each do |ext|
        $app_config["extpaths"].each do |p|
          extpath = File.join(p, ext, 'ext')
          if RUBY_PLATFORM =~ /(win|w)32$/
            next unless File.exists? File.join(extpath, 'build.bat')
          else
            next unless File.executable? File.join(extpath, 'build')
          end
          
          extroot = File.join(p,ext)

          extyml = File.join(extroot, "ext.yml")
          if File.file? extyml
            extconf = Jake.config(File.open(extyml))
            javaentry = extconf["javaentry"]
            extentries << javaentry unless javaentry.nil?
          end


          ext_tmp_dir = File.join(ENV["TARGET_TEMP_DIR"], ext) 
          ENV['TEMP_FILES_DIR'] = ext_tmp_dir
	      mkdir_p ext_tmp_dir unless File.directory? ext_tmp_dir

          if RUBY_PLATFORM =~ /(win|w)32$/
            puts Jake.run('build.bat', [], extpath)
          else
            puts Jake.run('./build', [], extpath)
          end
          exit 1 unless $?.success?

	  rm_rf ext_tmp_dir

        end
      end

      # finish build extensions

      # start adding builded extension jars to RhoBundle.jar

      # unpack all extensions JARs to extensionstmpdir	
      $app_config["extensions"].each do |ext|
        $app_config["extpaths"].each do |p|
          extpath = File.join(p, ext, 'ext')
          if RUBY_PLATFORM =~ /(win|w)32$/
            next unless File.exists? File.join(extpath, 'build.bat')
          else
            next unless File.executable? File.join(extpath, 'build')
          end
          Jake.unjar(extensionsdir + "/" + ext + ".jar",extensionstmpdir)
        end
      end

      Jake.unjar($bindir + "/RhoBundle.jar",extensionstmpdir)	

      rm_rf $bindir + "/RhoBundle.jar" if File.exists? $bindir + "/RhoBundle.jar"

      # repack new RhoBundle jar

      Jake.jar($bindir + "/RhoBundle.jar",extensionstmpdir + "/META-INF/manifest.mf",extensionstmpdir,true)

      rm_rf extensionsdir if File.exists? extensionsdir

      # finish adding builded extension jars to RhoBundle.jar 	

      #XXX make preverify function in Jake
      args = []
      #args << "-verbose"
      args << "-classpath"
      args << $bb_lib_path + ";"+$preverified+"/RubyVM.jar"
      args << "-d"
      args << $preverified
      args << $bindir + "/RhoBundle.jar"
      runPreverify(args)
      
      mkdir_p $rhobundledir unless File.exists? $rhobundledir
      cp $preverified + "/RhoBundle.jar", $rhobundledir + "/RhoBundle.jar"
      
    end

    task :set_dev_outname do
        $bundleClassName = "rhodes_"
    end
    
    task :devrhobundle => [:set_dev_outname,:rhobundle] do
      #cp $preverified + "/RhoBundle.jar", "platform/bb/RhoBundle/RhoBundle.jar"
	  cp $preverified + "/RhoBundle.jar", "platform/bb/Rhodes/RhoBundle.jar"
      
      sdcardpath = $bb_sim_path + "/sdcard/Rho/rhodes"
      
      namepath = File.join(sdcardpath,"name.txt")        
      old_appname = File.read(namepath) if File.exists?(namepath)
      if old_appname != $appname
        rm_rf sdcardpath 
        #This is eclipse bag : it creates rho folder under simulator
        rm_rf $bb_sim_path + "/Rho/rhodes"
      end
        
      mkdir_p sdcardpath unless File.exists?(sdcardpath)
      
      confpath = File.join( sdcardpath, "/apps/rhoconfig.txt" )
      rm_rf confpath if File.exists? confpath
      
      File.open(namepath, "w") { |f| f.write($appname) }      
      
    end

    def create_jarmanifest
      mf = File.join($builddir, "MANIFEST.MF")
      puts "mf: #{mf}"
      f = File.new(mf, "w")

      f.write "Manifest-Version: 1.0\n"
      f.write "MIDlet-Vendor: <unknown>\n"
      f.write "MIDlet-Version: 1.40\n"
      f.write "MicroEdition-Configuration: CLDC-1.1\n"
      f.write "MIDlet-1: ,resources/icon.png,\n"
      f.write "Created-By: 1.6.0_02 (Sun Microsystems Inc.)\n"
      f.write "MIDlet-Jar-URL: rhodes.jar\n"
      f.write "MIDlet-Name: rhodes\n"
      f.write "MicroEdition-Profile: MIDP-2.0\n"
      f.write "MIDlet-Jar-Size: 0\n"
      
      if $service_enabled      
        if $hidden_app == "0"
            f.write "RIM-MIDlet-Flags-1: 1\n" 
        else
            f.write "RIM-MIDlet-Flags-1: 3\n" 
        end    
      else
        if $hidden_app == "0"
            f.write "RIM-MIDlet-Flags-1: 0\n" 
        else
            f.write "RIM-MIDlet-Flags-1: 2\n" 
        end    
      end
      
      f.close
    
    end
    
    task :gensources => "config:bb" do
      caps = $app_config["capabilities"]
      caps = [] if caps.nil?
      caps = [] unless caps.is_a? Array

      has_push = caps.index("push") != nil

      $service_enabled = has_push
      create_jarmanifest()
        
      $stdout.flush

      f = StringIO.new("", "w+")      
      f.puts "package com.rho;"
      f.puts ""
      f.puts "public class Capabilities {"
      f.puts "  public static final boolean ENABLE_PUSH = #{has_push.to_s};"
      f.puts "  public static final boolean RUNAS_SERVICE = #{has_push.to_s};"
      f.puts "  public static final boolean USE_SQLITE = #{$use_sqlite.to_s};"
      f.puts "}"

      Jake.modify_file_if_content_changed( File.join($builddir, "..", "..", "..", "platform", "bb", "RubyVM", "src", "com", "rho", "Capabilities.java"), f )      
      extentries = []

      if $app_config["extensions"]
          $app_config["extensions"].each do |ext|
            $app_config["extpaths"].each do |p|
              extpath = File.join(p, ext, 'ext')
              if RUBY_PLATFORM =~ /(win|w)32$/
                next unless File.exists? File.join(extpath, 'build.bat')
              else
                next unless File.executable? File.join(extpath, 'build')
              end
              
              extroot = File.join(p,ext)

              extyml = File.join(extroot, "ext.yml")
              if File.file? extyml
                extconf = Jake.config(File.open(extyml))
                javaentry = extconf["javaentry"]
                extentries << javaentry unless javaentry.nil?
              end

            end
          end
      end

      f = StringIO.new("", "w+")      
      f.puts "// WARNING! THIS FILE IS GENERATED AUTOMATICALLY! DO NOT EDIT IT MANUALLY!"
      #f.puts "// Generated #{Time.now.to_s}"
      f.puts "package com.rho; "
      f.puts " "
      f.puts "public class Extensions {"
      f.puts " "
      f.puts "public static String[] extensions = {"

      extentries.each do |entry|
        f.puts '          "' + entry + '",'
      end

      f.puts '""'
      f.puts "};"
      f.puts " "
      f.puts "}"
      
      Jake.modify_file_if_content_changed(File.join($startdir,"/platform/bb/RubyVM/src/com/rho/Extensions.java"), f)

    end
    
#    desc "Build RubyVM"
    task :rubyvm => [:gensources, "config:bb"] do

#java -jar /Users/evgeny/Desktop/BBEclipse/plugins/net.rim.ejde.componentpack6.0.0_6.0.0.29/components/bin/rapc.jar -convertpng -quiet library=deliverables/Standard/6.0.0/RubyVM deliverables/Standard/6.0.0/RubyVM.rapc -exepath=/Users/evgeny/Desktop/BBEclipse/plugins/net.rim.ejde.componentpack6.0.0_6.0.0.29/components/bin/ -sourceroot=/Users/evgeny/Projects/rhodes/platform/bb/RubyVM/src:/Users/evgeny/Projects/rhodes/platform/bb/RubyVM/res -import=/Users/evgeny/Desktop/BBEclipse/plugins/net.rim.ejde.componentpack6.0.0_6.0.0.29/components/lib/net_rim_api.jar /Users/evgeny/Projects/rhodes/platform/bb/RubyVM/bin
      rubyvmfiles = File.readlines($builddir + '/RubyVM_build.files').map { |l| l.strip! }
        #f = File.new($builddir + '/RubyVM_build.files', "wb")
        #f.write(rubyvmfiles)
        #f.close        

	  
      if not uptodate?($preverified + '/RubyVM.jar',rubyvmfiles)
        puts "**********************Ruby VM is NOT Up to date!"
        mkdir_p $tmpdir + "/RubyVM" if not FileTest.exists? $tmpdir + "/RubyVM"
        mkdir_p  $targetdir if not FileTest.exists?  $targetdir
        mkdir_p  $preverified if not FileTest.exists?  $preverified

        args = []
        args << "-g"
        args << "-d"
        args << $tmpdir + '/RubyVM'
        args << "-bootclasspath"
        args << $bb_lib_path
        args << "-source"
        args << "1.3"
        args << "-target"
        args << "1.3"
        args << "-nowarn"
        args << "-sourcepath"		
        args << "#{$startdir}"				
        args << "@#{$builddir}/RubyVM_build.files"
        puts Jake.run($javac,args)
        unless $? == 0
          puts "Error compiling java code"
          exit 1
        end
        $stdout.flush
        #XXX Move to task/function
        args = []
        args << "-classpath"
        args << $bb_lib_path
        args << "-d"
        args << $tmpdir + "/RubyVM.preverify"
        args << $tmpdir + "/RubyVM"
        runPreverify(args)

        Jake.jar($preverified+"/RubyVM.jar", $builddir + "/RubyVM_manifest.mf", $tmpdir + "/RubyVM.preverify",true)
        $stdout.flush
      else
        puts 'RubyVM.jar is up to date'
        $stdout.flush
      end

      rm_rf $tmpdir
      mkdir_p $tmpdir
    end

   
#    desc "Build rhodes"
    task :rhodes => ["config:checkbb",:rubyvm, :rhobundle ] do

      sources = Dir.glob($builddir + "/../rhodes/resources/**/*") |
      ($use_sqlite ? [] : File.readlines($builddir + '/hsqldb_build.files').map { |l| l.strip! }) |
      File.readlines($builddir +'/rhodes_build.files').map { |l| l.strip! }

      sources.delete(nil)

      if not FileUtils.uptodate?($preverified + "/rhodes.jar",sources)

        $tmpdir.gsub!(/\\/, '/')
        vsrclist = $tmpdir + "/vsrc_build.files"

#        vsrclist = $builddir + "/../bin/vsrc_build.files"
#        mkdir_p $builddir + "/../bin" unless File.exists? $builddir + "/../bin"

        vsrcdir = $tmpdir + "/vsrc"
        mkdir_p vsrcdir
        cp_r $builddir + "/../rhodes/platform/common/.", vsrcdir
        #ar_bbver = $bbver.split('.')
        [ "4.7", "5.0" ].each do |ver|
            if $bbver > ver    
                if File.exist?( $builddir + "/../rhodes/platform/" + ver )
                  cp_r $builddir + "/../rhodes/platform/" + ver + "/.", vsrcdir, :remove_destination => true
                end
            end
        end
        
        if File.exist?( $builddir + "/../rhodes/platform/" + $bbver )
          cp_r $builddir + "/../rhodes/platform/" + $bbver + "/.", vsrcdir, :remove_destination => true
        end

        # Modify sources to get different class names due to BB limitation -
        # there can not be two or more applications installed which contains the same
        # class names which implements Persistable interface. See details here -
        # http://supportforums.blackberry.com/rim/board/message?board.id=java_dev&thread.id=11152
        mkdir_p vsrcdir + "/com/rho/file"
        cp_r $builddir + "/../rhodes/src/com/rho/file/PersistRAFileImpl.java", vsrcdir + "/com/rho/file"
        freplace( vsrcdir + "/com/rho/file/PersistRAFileImpl.java", /FileInfoWrapper/, "_" + $outfilebase + "_FileInfoWrapper" )
        freplace( vsrcdir + "/com/rho/file/PersistRAFileImpl.java", /PageWrapper/, "_" + $outfilebase + "_PageWrapper" )

        fvsrc = File.new( vsrclist, "w" )
        Dir.glob( vsrcdir + "/**/*.java" ).each do |line|
          fvsrc.puts "\""+line+"\""
        end
        fvsrc.close

        args = []
        args << "-g"
        args << "-d"
        args << $tmpdir
        args << "-classpath"
        args << $bindir + "/RhoBundle.jar;"+$preverified+"/RubyVM.jar"
        args << "-bootclasspath"
        args << $bb_lib_path
        args << "-source"
        args << "1.3"
        args << "-target"
        args << "1.3"
        args << "-nowarn"
        args << "@#{vsrclist}"
        #args << "@RubyVM_build.files"
        args << "@#{$builddir}/hsqldb_build.files" if !$use_sqlite
        args << "@#{$builddir}/rhodes_build.files"
        puts "\texecuting javac"
        puts Jake.run($javac,args)
        unless $? == 0
          puts "Error compiling java code"
          exit 1
        end
        $stdout.flush

        rm_rf vsrcdir

        cp_r $builddir + "/../rhodes/resources", $tmpdir + "/resources"
        cp $app_path + "/icon/icon.png", $tmpdir +"/resources"
        
        Jake.jar($bindir + "/rhodes.jar", $builddir + "/manifest.mf", $tmpdir,true)
        $stdout.flush
        args = []
        args << "-classpath"
        args << $rhodesimplib
        args << "-d"
        args << $preverified
        args << $bindir + "/rhodes.jar"
        runPreverify(args)
      else
        puts "rhodes up to date"
      end

    end
  end
end

namespace "package" do
  namespace "bb" do
#    desc "Package rhoBundle"
    task :rhobundle => ["build:bb:rhobundle"] do
      Jake.rapc("RhoBundle",
        $targetdir,
        $rhobundleimplib ,
        $preverified + "/RhoBundle.jar",
        "RhoBundle",
        $app_config["vendor"],
        $app_config["version"]
      )
      unless $? == 0
        puts "Error in RAPC"
        exit 1
      end
      
      create_alx_file('RhoBundle', 'RhoBundle')
    end

#    desc "Package rubyVM"
    task :rubyvm => "build:bb:rubyvm" do

      if not FileUtils.uptodate?($targetdir + '/RubyVM.cod',$preverified + "/RubyVM.jar")
        Jake.rapc("RubyVM",
          $targetdir,
          $bb_lib_path,
          $preverified + "/RubyVM.jar",
          "RubyVM",
          $app_config["vendor"],
          $app_config["version"]
        )
        unless $? == 0
          puts "Error in RAPC"
          exit 1
        end
        $stdout.flush
      else
        puts 'RubyVM .cod files are up to date'
        $stdout.flush
      end

    end

#    desc "Package rhodesApp"
    task :rhodes => ["build:bb:rhodes"] do
      if not FileUtils.uptodate?($targetdir + '/' + $outfilebase + '.cod',$preverified + "/rhodes.jar")
        Jake.rapc($outfilebase,
          $targetdir,
          $rhodesimplib,
          $preverified + "/rhodes.jar",
          $appname,
          $app_config["vendor"],
          $app_config["version"],
          "resources/icon.png",
          false,
          true
        )
        unless $? == 0
          puts "Error in RAPC"
          exit 1
        end
        $stdout.flush
        
        create_alx_file('rhodesApp', $outfilebase)
        
      else
        puts 'rhodes .cod files are up to date'
        $stdout.flush
      end
          
    end
    
    def runProGuard(target_jar, lib_jar)
	  
      proguard =  File.join( $startdir, '/res/build-tools/proguard-4.5.1.jar' )
      out_jar = File.join( File.dirname(target_jar), File.basename(target_jar, File.extname(target_jar) ) + "_out.jar" )
      puts "****out jar: #{out_jar}"
      
      args = []
      args << "-jar"
      args << proguard
      args << "-injars"
      args << target_jar
      args << "-outjars"
      args << out_jar
      args << "-libraryjars"
      args << $bb_lib_path
      args << "-overloadaggressively"
      args << "-repackageclasses '' "
      args << "-allowaccessmodification"
      args << "-microedition"
      args << "-printseeds"
      args << "-dontusemixedcaseclassnames"
      #args << "-keep public class * extends net.rim.device.api.ui.UiApplication"
      #args << "-keep final public class rhomobile.RhodesApplication"
      args << "-keep final public class rhomobile.RhodesApplication { public static void main(java.lang.String[]); }"
      #args << "-keep public class xruby.version.main { public static void main(java.lang.String[]); }"
      #args << "-keep public class xruby.rhoframework.main { public static void main(java.lang.String[]); }"
      
      #args << "-keepclasseswithmembers public class * {  public static void main(java.lang.String[]); }"
      #args << "-dontobfuscate"
      #args << "-dontpreverify"
      #args << "-target 1.3"
      args << "-dontshrink"
      args << "-dontoptimize"
      
      puts Jake.run(java,args)
      $stdout.flush
      
      rm_rf target_jar
      File.rename( out_jar, target_jar )
    end

#    desc "Package all production (all parts in one package)"
    task :production => ["build:bb:rhodes"] do
      rm_rf $tmpdir
      mkdir_p $tmpdir

      rm_rf $targetdir
      mkdir_p $targetdir

      Jake.unjar($preverified + "/RubyVM.jar", $tmpdir)
      Jake.unjar($preverified + "/RhoBundle.jar", $tmpdir)
      Jake.unjar($preverified + "/rhodes.jar", $tmpdir)

      if $target_sim
          #Changing rhoconfig.txt to work on Windows 7
          if RUBY_PLATFORM =~ /(win|w)32$/
	        require  'win32/registry'

	        def getWindowsProductNameString
	            Win32::Registry::HKEY_LOCAL_MACHINE.open('SOFTWARE\Microsoft\Windows NT\CurrentVersion') do |reg|
	                reg_typ, reg_val = reg.read('ProductName')
	                return reg_val
	            end
	        end

      	    if getWindowsProductNameString =~ /Windows 7/
	            fixWin7SimBug($tmpdir + "/apps/rhoconfig.txt")	
	        end
          end
      end
      
      if $bbver =~ /^4\.[012](\..*)$/
        max_size = 65536
        Dir.glob( $tmpdir + "/**/*" ).each do |f|
          if File.size( f ) > max_size
            puts "File size of " + f + " is more than " + max_size.to_s + " bytes"
            puts "There is no ability to pack this file into .cod file for BB " + $bbver
            puts "Please reduce its size and try again"
            $stdout.flush
            Process.exit
          end
        end
      end

      Jake.jar($bindir + "/" + $outfilebase + ".jar",$builddir + "/manifest.mf",$tmpdir,true)
      
      Jake.rapc($outfilebase,
        $targetdir,
        $bb_lib_path,
        $bindir + "/" + $outfilebase + ".jar",
        $appname,
        $app_config["vendor"],
        $app_config["version"],
        "resources/icon.png",
        false,
        true
      )
      unless $? == 0
        puts "Error in RAPC"
        exit 1
      end
      $stdout.flush

      File.open(File.join($targetdir,"app_info.txt"), "w") { |f| f.write( $outfilebase ) }
      
      create_alx_file('rhodesApp', $outfilebase)
    end

    task :set_simulator do
        $target_sim = true
    end
    
    desc "Package all production (all parts in one package) for simulator"
    task :production_sim => [:set_simulator, :production] do
    end

    task :set_dev_build do
        $dev_build = true
    end
    
#    desc "Package all dev (each part in separate package)"
    task :dev => [ :set_dev_build, :rubyvm,:rhobundle,:rhodes] do
    end
    
  end
end

namespace "device" do
  namespace "bb" do
    desc "Build debug for device"
    task :debug => "package:bb:dev" do

      #make into functions
      if $config["build"]["bbsignpwd"] and $config["build"]["bbsignpwd"] != ""
        autosign
      else
        manualsign
      end

      rm_rf $targetdir + "/web"
      mkdir_p $targetdir + "/web"

      cp $targetdir + "/RhoBundle.jad", $targetdir + "/web"
      cp $targetdir + "/"+$outfilebase+".jad", $targetdir + "/web"
      cp $targetdir + "/RubyVM.jad", $targetdir + "/web"

      Jake.unjar($targetdir + "/RhoBundle.cod", $targetdir + "/web")
      Jake.unjar($targetdir + "/"+$outfilebase+".cod", $targetdir + "/web")
      Jake.unjar($targetdir + "/RubyVM.cod", $targetdir + "/web")

    end

#    desc "Build and package dev rhobundle for device"
    task :rhobundle => "package:bb:rhobundle" do

      if $config["build"]["bbsignpwd"] and $config["build"]["bbsignpwd"] != ""
        autosign
      else
        manualsign
      end

      rm_rf Dir.glob($targetdir + "/web/RhoBundle*.*")

      cp $targetdir + "/RhoBundle.jad", $targetdir + "/web"

      Jake.unjar($targetdir + "/RhoBundle.cod", $targetdir + "/web")

    end

    desc "Build production for device"
    task :production => "package:bb:production" do

      if $config["build"]["bbsignpwd"] and $config["build"]["bbsignpwd"] != ""
        autosign
      else
        manualsign
      end

      rm_rf $targetdir + "/ota-web"
      mkdir_p $targetdir + "/ota-web"

      cp $targetdir + "/"+$outfilebase+".jad", $targetdir + "/ota-web"

      Jake.unjar($targetdir + "/"+$outfilebase+".cod", $targetdir + "/ota-web")
      
      rm_rf Dir.glob($targetdir + "/*.debug")

    end

  end
end

namespace "clean" do
  desc "Clean bb"
  task :bb => "clean:bb:all"

  desc "Clean bb and simulator"
  task :bbsim => ["clean:bb:all"] do

      command =  "clean.bat"
      args = []
      Jake.run2 command, args, {:directory => $bb_sim_path, :system => true}
      
      rm_rf $bb_sim_path + "/sdcard/Rho" if File.exists?($bb_sim_path + "/sdcard/Rho")
  end
  
  namespace "bb" do
    task :all => ["config:bb"] do

      rm_rf $bindir +"/preverified" if File.exists? $bindir +"/preverified"
      rm_rf $bindir +"/RhoBundle" if File.exists? $bindir +"/RhoBundle"
      rm_rf $bindir +"/tmp" if File.exists? $bindir +"/tmp"
      rm_rf $targetdir if File.exists? $targetdir
      
      rm_rf Dir.glob($bindir + "/*.jar")
    end

  end
end

namespace "run" do
  namespace "bb" do

      def getLogFilePath
        return $bb_sim_path + "/sdcard/Rho/" + $outfilebase + "/RhoLog.txt"
      end
      # >>>>>>>>>>>>>>>>>>>>>
      task :get_log => ["config:bb"] do
         puts "log_file=" + getLogFilePath
      end

      task :stopmdsandsim => ["config:bb"] do
        stopsim  
        stopmds
      end

      task :stopmds => ["config:bb"] do
        stopmds
      end

#      desc "Starts mds and sim"
      task :startmdsandsim => ["config:bb"] do
        startmds
        startsim
      end

      task :spec => ["config:checkbb", "run:bb:stopmdsandsim", "clean:bbsim", "package:bb:production_sim"] do
        cp_r File.join($targetdir,"/."), $bb_sim_path
        #rm_rf $bb_sim_path + "/sdcard/Rho"

        log_name = $bb_sim_path + "/sdcard/Rho/" + $outfilebase + "/RhoLog.txt"
        puts log_name
        #log_name  = Jake.get_absolute($app_config["applog"] )
        #File.delete(log_name) if File.exist?(log_name)
        
        startmds
        startsim(true)

        Jake.before_run_spec
        start = Time.now

        while !File.exist?(log_name)
            sleep(1)
        end

        io = File.new(log_name, "r")
        waiting_count = 0
        end_spec = false
        while !end_spec do
            line_count = 0
            io.each do |line|
                #puts line
                end_spec = !Jake.process_spec_output(line)
                break if end_spec
                line_count += 1
            end
            if line_count==0
                waiting_count += 1
            else
                waiting_count = 0
            end
            if waiting_count > 240
                puts "spec application hung (240 seconds timeout)"
                end_spec = true
            end
            sleep(1) unless end_spec
        end
        io.close
        
        stopsim  
        stopmds

        Jake.process_spec_results(start)        
        
        $stdout.flush
        
      end

      task :phone_spec do
        Jake.run_spec_app('bb','phone_spec')
        exit 1 if $total.to_i==0
        exit $failed.to_i
      end

    task :testsim => ["config:bb"] do
      load_to_sim(false)
    end

    desc "Start Blackberry simulator"
    task :startsim => ["config:bb"] do
      startsim
    end  

    task :stopmdsandsim_ex => ["config:bb"] do
        
        stopsim if $bbver.to_f < 5.0
        stopmds
    end  

    desc "Run application on RhoSimulator"    
    task :rhosimulator => ["config:set_bb_platform", "config:common", "config:read_bb_version"] do
    
        if $bb6
            $rhosim_config = "platform='bb6'\r\n"
        else
            $rhosim_config = "platform='bb'\r\n"            
        end    

        $rhosim_config += "os_version='#{$bbver}'\r\n"
        
        Rake::Task["run:rhosimulator"].invoke
    end

    task :rhosimulator_debug => ["config:set_bb_platform", "config:common", "config:read_bb_version"] do
    
        if $bb6
            $rhosim_config = "platform='bb6'\r\n"
        else
            $rhosim_config = "platform='bb'\r\n"            
        end    

        $rhosim_config += "os_version='#{$bbver}'\r\n"
        
        Rake::Task["run:rhosimulator_debug"].invoke
    end
    
  end

  desc "Builds everything, loads and starts bb sim and mds"
  task :bb => ["run:bb:stopmdsandsim", "package:bb:production_sim"] do

    cp_r File.join($targetdir,"/."), $bb_sim_path
    
    startmds
    startsim 
    
    $stdout.flush
  end

  desc "Builds everything and loads application on simulator"
  task :bbapp => ["run:bb:stopmdsandsim_ex", "package:bb:production_sim"] do
    
    startmds

    if $bbver.to_f < 5.0
      cp_r File.join($targetdir,"/."), $bb_sim_path
      startsim 
    elsif !load_to_sim(false)
      cp_r File.join($targetdir,"/."), $bb_sim_path
      
      startsim 
      
    end

    $stdout.flush
  end

  desc "Same as run:bb, but only supports one app at a time and works faster"
  task :bbdev => ["run:bb:stopmdsandsim", "package:bb:dev"] do
    
    cp_r File.join($targetdir,"/."), $bb_sim_path
    
    startmds
    startsim
    $stdout.flush
  end
  
end

namespace "config" do
  desc "Check local blackberry configuration"
  task :checkbb => ["config:bb"] do
    javahome = $config["env"]["paths"]["java"]
    mdshome  = $config["env"]["paths"][$bbver]["mds"]

    begin
      puts Jake.run('javac', [], nil, false)
    rescue
      puts "Java directory does not in your PATH enviroment variable."
      throw "main JAVA directory missing"
    end

#    if $retJava == nil or $retJava == ""
#      puts "Java directory does not in your PATH enviroment variable."
#      throw "main JAVA directory missing"
#    end

    puts "BBVER: " + $bbver
    puts "JAVAHOME: " + javahome
    puts "JDEHOME: " + $bb_jde_path
    puts "MDSHOME: " + mdshome

    if not FileTest.exists? javahome
      puts "JAVAHOME does not exist. Make sure you have the Java SDK installed and that build.yml has the correct path"
      throw "JAVAHOME missing"
    end

    if not FileTest.exists? $javac
      puts "#{$javac} not found. Make sure JAVAHOME points to a valid Java SDK"
      throw "javac missing"
    end

    if not FileTest.exists? javahome + "/java.exe"
      puts "java.exe not found. Make sure JAVAHOME points to a valid Java SDK"
      throw "java missing"
    end

    if not FileTest.exists? javahome + "/jar.exe"
      puts "jar.exe not found. Make sure JAVAHOME points to a valid Java SDK"
      throw "jar missing"
    end

    if not FileTest.exists? $bb_jde_path
      puts "JDEHOME does not exist. Make sure you have the Blackberry JDK installed and that build.yml has the correct path"
      throw "JDEHOME missing"
    end
    if not FileTest.exists? mdshome
      puts "MDSHOME does not exist. Make sure you have the Blackberry JDK installed and that build.yml has the correct path"
      throw "MDSHOME missing"
    end

    if not FileTest.exists? $bb_jde_path + "/bin/preverify.exe"
      puts "preverify.exe not found. Make sure JDEHOME points to a valid Blackberry JDK"
      throw "preverify missing"
    end

    if not FileTest.exists? $bb_jde_path + "/bin/rapc.jar"
      puts "rapc.jar not found. Make sure JDEHOME points to a valid Blackberry JDK"
      throw "rapc missing"
    end

    puts "Config appears valid"
  end

end
