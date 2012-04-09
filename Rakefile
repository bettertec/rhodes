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

require 'find'
require 'erb'
#require 'rdoc/task'
require 'digest/sha2'
require 'rexml/document'

#Look, another big fat hack. Make it so we can remove tasks from rake -T by setting comment to nil
module Rake
  class Task
    attr_accessor :comment
  end
end

$app_basedir = pwd
chdir File.dirname(__FILE__)

require File.join(pwd, 'lib/build/jake.rb')

load File.join(pwd, 'platform/bb/build/bb.rake')
load File.join(pwd, 'platform/android/build/android.rake')
load File.join(pwd, 'platform/iphone/rbuild/iphone.rake')
load File.join(pwd, 'platform/wm/build/wm.rake')
load File.join(pwd, 'platform/linux/tasks/linux.rake')
load File.join(pwd, 'platform/wp7/build/wp.rake')
load File.join(pwd, 'platform/symbian/build/symbian.rake')
load File.join(pwd, 'platform/osx/build/osx.rake')

def get_dir_hash(dir, init = nil)
  hash = init
  hash = Digest::SHA2.new if hash.nil?
  Dir.glob(dir + "/**/*").each do |f|
    hash << f
    hash.file(f) if File.file? f
  end
  hash
end

namespace "framework" do
  task :spec do
    loadpath = $LOAD_PATH.inject("") { |load_path,pe| load_path += " -I" + pe }

    rhoruby = ""

    if RUBY_PLATFORM =~ /(win|w)32$/
      rhoruby = 'res\\build-tools\\RhoRuby'
    elsif RUBY_PLATFORM =~ /darwin/
      rhoruby = 'res/build-tools/RubyMac'
    else
      rhoruby = 'res/build-tools/rubylinux'
    end
   
    puts `#{rhoruby}  -I#{File.expand_path('spec/framework_spec/app/')} -I#{File.expand_path('lib/framework')} -I#{File.expand_path('lib/test')} -Clib/test framework_test.rb`
  end
end


$application_build_configs_keys = ['security_token', 'encrypt_database', 'android_title', 'iphone_db_in_approot', 'iphone_set_approot', 'iphone_userpath_in_approot']

def make_application_build_config_header_file
  f = StringIO.new("", "w+")      
  f.puts "// WARNING! THIS FILE IS GENERATED AUTOMATICALLY! DO NOT EDIT IT MANUALLY!"
  #f.puts "// Generated #{Time.now.to_s}"
  f.puts ""
  f.puts "#include <string.h>"
  f.puts ""
  f.puts '#include "app_build_configs.h"'
  f.puts ""
      
  f.puts 'static const char* keys[] = { ""'
  $application_build_configs.keys.each do |key|
    f.puts ',"'+key+'"'
  end
  f.puts '};'
  f.puts ''
  
  count = 1

  f.puts 'static const char* values[] = { ""'
  $application_build_configs.keys.each do |key|
    f.puts ',"'+$application_build_configs[key].to_s+'"'
    count = count + 1
  end
  f.puts '};'
  f.puts ''

  f.puts '#define APP_BUILD_CONFIG_COUNT '+count.to_s
  f.puts ''
  f.puts 'const char* get_app_build_config_item(const char* key) {'
  f.puts '  int i;'
  f.puts '  for (i = 1; i < APP_BUILD_CONFIG_COUNT; i++) {'
  f.puts '    if (strcmp(key, keys[i]) == 0) {'
  f.puts '      return values[i];'
  f.puts '    }'
  f.puts '  }'
  f.puts '  return 0;'
  f.puts '}'
  f.puts ''
  
  Jake.modify_file_if_content_changed(File.join($startdir, "platform", "shared", "common", "app_build_configs.c"), f)
end

def make_application_build_capabilities_header_file
  f = StringIO.new("", "w+")      
  f.puts "// WARNING! THIS FILE IS GENERATED AUTOMATICALLY! DO NOT EDIT IT MANUALLY!"
  #f.puts "// Generated #{Time.now.to_s}"
  f.puts ""

  caps = []
 
  capabilities = $app_config["capabilities"]

  if capabilities != nil && capabilities.is_a?(Array)
     capabilities.each do |cap|
        caps << cap
     end
  end

  caps.sort.each do |cap|
     f.puts '#define APP_BUILD_CAPABILITY_'+cap.upcase
  end

  f.puts ''
  
  Jake.modify_file_if_content_changed(File.join($startdir, "platform", "shared", "common", "app_build_capabilities.h"), f)
end

def make_application_build_config_java_file

    f = StringIO.new("", "w+")
    f.puts "// WARNING! THIS FILE IS GENERATED AUTOMATICALLY! DO NOT EDIT IT MANUALLY!"
    #f.puts "// Generated #{Time.now.to_s}"

    f.puts "package com.rho;"
    f.puts ""
    f.puts "public class AppBuildConfig {"

    f.puts 'static final String keys[] = { ""'
    $application_build_configs.keys.each do |key|
      f.puts ',"'+key+'"'
    end
    f.puts '};'
    f.puts ''

    count = 1

    f.puts 'static final String values[] = { ""'
    $application_build_configs.keys.each do |key|
      f.puts ',"'+$application_build_configs[key]+'"'
      count = count + 1
    end
    f.puts '};'
    f.puts ''

    f.puts 'static final int APP_BUILD_CONFIG_COUNT = '+count.to_s + ';'
    f.puts ''
    f.puts 'public static String getItem(String key){'
    f.puts '  for (int i = 1; i < APP_BUILD_CONFIG_COUNT; i++) {'
    f.puts '    if ( key.compareTo( keys[i]) == 0) {'
    f.puts '      return values[i];'
    f.puts '    }'
    f.puts '  }'
    f.puts '  return null;'
    f.puts '}'
    f.puts "}"

    Jake.modify_file_if_content_changed( File.join( $startdir, "platform/bb/RubyVM/src/com/rho/AppBuildConfig.java" ), f )
end

namespace "config" do
  task :common do
    $startdir = File.dirname(__FILE__)
    $startdir.gsub!('\\', '/')
    
    $binextensions = []
    buildyml = 'rhobuild.yml'

    buildyml = ENV["RHOBUILD"] unless ENV["RHOBUILD"].nil?
    $config = Jake.config(File.open(buildyml))
    $config["platform"] = $current_platform if $current_platform
    
    if RUBY_PLATFORM =~ /(win|w)32$/
      $all_files_mask = "*.*"
      $rubypath = "res/build-tools/RhoRuby.exe"
    else
      $all_files_mask = "*"
      if RUBY_PLATFORM =~ /darwin/
        $rubypath = "res/build-tools/RubyMac"
      else
        $rubypath = "res/build-tools/rubylinux"
      end
    end
	
    if $app_path.nil? #if we are called from the rakefile directly, this wont be set
      #load the apps path and config

      $app_path = $config["env"]["app"]
      unless File.exists? $app_path
        puts "Could not find rhodes application. Please verify your application setting in #{File.dirname(__FILE__)}/rhobuild.yml"
        exit 1
      end
    end

    ENV["ROOT_PATH"] = $app_path.to_s + '/app/'
    ENV["APP_TYPE"] = "rhodes"

    $app_config = Jake.config(File.open(File.join($app_path, "build.yml")))

    Jake.set_bbver($app_config["bbver"].to_s)
    
    extpaths = []

    extpaths << $app_config["paths"]["extensions"] if $app_config["paths"] and $app_config["paths"]["extensions"]
    extpaths << $config["env"]["paths"]["extensions"] if $config["env"]["paths"]["extensions"]
    extpaths << File.join($app_path, "extensions")
    extpaths << File.join($startdir, "lib","extensions")
    $app_config["extpaths"] = extpaths

    if $app_config["build"] and $app_config["build"] == "release"
      $debug = false
    else
      $debug = true
    end
    
    extensions = []
    extensions += $app_config["extensions"] if $app_config["extensions"] and
       $app_config["extensions"].is_a? Array
    extensions += $app_config[$config["platform"]]["extensions"] if $app_config[$config["platform"]] and
       $app_config[$config["platform"]]["extensions"] and $app_config[$config["platform"]]["extensions"].is_a? Array
    $app_config["extensions"] = extensions
    
    capabilities = []
    capabilities += $app_config["capabilities"] if $app_config["capabilities"] and
       $app_config["capabilities"].is_a? Array
    capabilities += $app_config[$config["platform"]]["capabilities"] if $app_config[$config["platform"]] and
       $app_config[$config["platform"]]["capabilities"] and $app_config[$config["platform"]]["capabilities"].is_a? Array
    $app_config["capabilities"] = capabilities

    application_build_configs = {}

    #Process rhoelements settings
    if $app_config["app_type"] == 'rhoelements'
        $app_config["capabilities"] += ["motorola"] unless $app_config["capabilities"].index("motorola")
        $app_config["extensions"] += ["rhoelementsext"] if $current_platform == 'wm'
        $app_config["extensions"] += ["motoapi"] #extension with plug-ins
        $app_config["extensions"] += ['webkit-browser'] unless $app_config["extensions"].index("webkit-browser")
        
        #check for RE2 plugins
        plugins = ""
        $app_config["extensions"].each do |ext|
            if ( ext.start_with?('moto-') )
                plugins += ',' if plugins.length() > 0
                plugins += ext[5, ext.length()-5]
            end
        end
        
        if plugins.length() == 0
            plugins = "ALL"    
        end
        
        application_build_configs['moto-plugins'] = plugins if plugins.length() > 0
        
    end
    
    if $app_config["capabilities"].index("motorola")
        if $app_config["extensions"].index("webkit-browser")
            $app_config["capabilities"] += ["webkit_browser"]
            $app_config["extensions"].delete("webkit-browser") unless $current_platform == 'android'
        end
        if $current_platform == 'android'
            barcode_idx = $app_config['extensions'].index('barcode')
            $app_config['extensions'][barcode_idx] = 'barcode-moto' unless barcode_idx.nil?
        end
        $app_config["extensions"] += ["rhoelements"]
    end

    puts "$app_config['extensions'] : #{$app_config['extensions'].inspect}"   
    puts "$app_config['capabilities'] : #{$app_config['capabilities'].inspect}"   
    
    $hidden_app = $app_config["hidden_app"].nil?() ? "0" : $app_config["hidden_app"]
    
    #application build configs

    $application_build_configs_keys.each do |key|
      value = $app_config[key]
      if $app_config[$config["platform"]] != nil
        if $app_config[$config["platform"]][key] != nil
          value = $app_config[$config["platform"]][key]
        end
      end
      if value != nil
        application_build_configs[key] = value
      end
    end	
    $application_build_configs = application_build_configs

    if $current_platform == "bb"  
      make_application_build_config_java_file
    else  
      make_application_build_config_header_file    
      make_application_build_capabilities_header_file
    end

    $rhologhostport = $config["log_host_port"] 
    $rhologhostport = 52363 unless $rhologhostport
	$rhologhostaddr = Jake.localip()

    $obfuscate_js = (($app_config["obfuscate"].nil? || $app_config["obfuscate"]["js"].nil?) ? nil : 1 )
    $obfuscate_css = (($app_config["obfuscate"].nil? || $app_config["obfuscate"]["css"].nil?) ? nil : 1 )
    $obfuscate_exclude = ($app_config["obfuscate"].nil? ? nil : $app_config["obfuscate"]["exclude_dirs"] )
    $obfuscator = 'res/build-tools/yuicompressor-2.4.7.jar'
  end

  task :qt do
    $qtdir = ENV['QTDIR']
    unless (!$qtdir.nil?) and ($qtdir !~/^\s*$/) and File.directory?($qtdir)
      puts "\nPlease, set QTDIR environment variable to Qt root directory path"
      exit 1
    end
    $qmake = File.join($qtdir, 'bin/qmake')
    $macdeployqt = File.join($qtdir, 'bin/macdeployqt')
  end

  out = `javac -version 2>&1`
  puts "\n\nYour java bin folder does not appear to be on your path.\nThis is required to use rhodes.\n\n" unless $? == 0
end

def copy_assets(asset)
  
  dest = File.join($srcdir,'apps/public')
  
  cp_r asset + "/.", dest, :preserve => true, :remove_destination => true 
  
end

def clear_linker_settings
  if $config["platform"] == "iphone"
#    outfile = ""
#    IO.read($startdir + "/platform/iphone/rhorunner.xcodeproj/project.pbxproj").each_line do |line|
#      if line =~ /EXTENSIONS_LDFLAGS = /
#        outfile << line.gsub(/EXTENSIONS_LDFLAGS = ".*"/, 'EXTENSIONS_LDFLAGS = ""')
#      else
#        outfile << line
#      end
#    end
#    File.open($startdir + "/platform/iphone/rhorunner.xcodeproj/project.pbxproj","w") {|f| f.write outfile}
#    ENV["EXTENSIONS_LDFLAGS"] = ""

    $ldflags = ""
  end

end

def add_linker_library(libraryname)
#  if $config["platform"] == "iphone"
#    outfile = ""
#    IO.read($startdir + "/platform/iphone/rhorunner.xcodeproj/project.pbxproj").each_line do |line|
#      if line =~ /EXTENSIONS_LDFLAGS = /
#        outfile << line.gsub(/";/, " $(TARGET_TEMP_DIR)/#{libraryname}\";")
#      else
#        outfile << line
#      end
#    end
#    File.open($startdir + "/platform/iphone/rhorunner.xcodeproj/project.pbxproj","w") {|f| f.write outfile}
#  end
      simulator = $sdk =~ /iphonesimulator/

      if ENV["TARGET_TEMP_DIR"] and ENV["TARGET_TEMP_DIR"] != ""
        tmpdir = ENV["TARGET_TEMP_DIR"]
      else
        tmpdir = $startdir + "/platform/iphone/build/rhorunner.build/#{$configuration}-" +
          ( simulator ? "iphonesimulator" : "iphoneos") + "/rhorunner.build"
      end
  $ldflags << "#{tmpdir}/#{libraryname}\n" unless $ldflags.nil?
end

def set_linker_flags
  if $config["platform"] == "iphone"
      simulator = $sdk =~ /iphonesimulator/
      if ENV["TARGET_TEMP_DIR"] and ENV["TARGET_TEMP_DIR"] != ""
        tmpdir = ENV["TARGET_TEMP_DIR"]
      else
        tmpdir = $startdir + "/platform/iphone/build/rhorunner.build/#{$configuration}-" +
          ( simulator ? "iphonesimulator" : "iphoneos") + "/rhorunner.build"
      end
      mkdir_p tmpdir unless File.exist? tmpdir
      File.open(tmpdir + "/rhodeslibs.txt","w") { |f| f.write $ldflags }
#    ENV["EXTENSIONS_LDFLAGS"] = $ldflags
#    puts `export $EXTENSIONS_LDFLAGS`
  end

end

def add_extension(path,dest)
  puts 'add_extension - ' + path.to_s + " - " + dest.to_s
  
  start = pwd
  chdir path if File.directory?(path)

  Dir.glob("*").each { |f| cp_r f,dest unless f =~ /^ext(\/|(\.yml)?$)/ }

  chdir start
end

def init_extensions(startdir, dest)
  extentries = []
  nativelib = []
  extlibs = [] 
  extpaths = $app_config["extpaths"]  
    
  puts 'init extensions'
  $app_config["extensions"].each do |extname|  
    puts 'ext - ' + extname
    
    extpath = nil
    extpaths.each do |p|
      if p
      ep = File.join(p, extname)
      if File.exists? ep
        extpath = ep
        break
      end
      end
    end    
    
    puts '1'

    if extpath.nil?
      begin
        $rhodes_extensions = nil
        require extname
        puts '1-2'
        if $rhodes_extensions
            extpath = $rhodes_extensions[0]
            $app_config["extpaths"] << extpath
        end    
      rescue Exception => e      
        puts "exception"  
      end
    end
        
    unless extpath.nil?      
      add_extension(extpath, dest) unless dest.nil?

      if $config["platform"] != "bb"
        extyml = File.join(extpath, "ext.yml")
        puts "extyml " + extyml 
        
        if File.file? extyml
          extconf = Jake.config(File.open(extyml))
          entry = extconf["entry"]
          nlib = extconf["nativelibs"]
          type = extconf["exttype"]
            
          if nlib != nil
            nlib.each do |libname|
              nativelib << libname
            end
          end
          
          extentries << entry unless entry.nil?
          
          if type.to_s() != "nativelib"
            libs = extconf["libraries"]
            libs = [] unless libs.is_a? Array
            if $config["platform"] == "wm" || $config["platform"] == "win32"
              libs.map! { |lib| lib + ".lib" }
            else
              libs.map! { |lib| "lib" + lib + ".a" }
            end
            extlibs += libs
          end
        end
      end
      
    end
    
  end
  
  exts = File.join($startdir, "platform", "shared", "ruby", "ext", "rho", "extensions.c")
  puts "exts " + exts
  
  if $config["platform"] != "bb"
    exists = []
      
    if ( File.exists?(exts) )
      File.new(exts, "r").read.split("\n").each do |line|
        next if line !~ /^\s*extern\s+void\s+([A-Za-z_][A-Za-z0-9_]*)/
        exists << $1
      end
    end
  
    #if (exists.sort! != extentries.sort! ) || (!File.exists?(exts))
      File.open(exts, "w") do |f|
        puts "MODIFY : #{exts}"
          
        f.puts "// WARNING! THIS FILE IS GENERATED AUTOMATICALLY! DO NOT EDIT IT MANUALLY!"
        #f.puts "// Generated #{Time.now.to_s}"
        if $config["platform"] == "wm" || $config["platform"] == "win32"
          # Add libraries through pragma
          extlibs.each do |lib|
            f.puts "#pragma comment(lib, \"#{lib}\")"
          end

          nativelib.each do |lib|
            f.puts "#pragma comment(lib, \"#{lib}\")"
          end
        end
          
        extentries.each do |entry|
          f.puts "extern void #{entry}(void);"
        end
    
        f.puts "void Init_Extensions(void) {"
        extentries.each do |entry|
          f.puts "    #{entry}();"
        end
        f.puts "}"
      end
    #end

    extlibs.each { |lib| add_linker_library(lib) }
    nativelib.each { |lib| add_linker_library(lib) }

    set_linker_flags
    
    #exit
  end
  
  unless $app_config["constants"].nil?
    File.open("rhobuild.rb","w") do |file|
      file << "module RhoBuild\n"
      $app_config["constants"].each do |key,value|
        value.gsub!(/"/,"\\\"")
        file << "  #{key.upcase} = \"#{value}\"\n"
      end
      file << "end\n"
    end
  end

  if $excludeextlib and (not dest.nil?)
      chdir dest
      $excludeextlib.each {|e| Dir.glob(e).each {|f| rm f}}
  end

end

def public_folder_cp_r(src_dir,dst_dir,level,obfuscate)
  mkdir_p dst_dir if not File.exists? dst_dir
  Dir.foreach(src_dir) do |filename|
    next if filename.eql?('.') || filename.eql?('..')
    filepath = src_dir + '/' + filename
    dst_path = dst_dir + '/' + filename
    if File.directory?(filepath)
      public_folder_cp_r(filepath,dst_path,(level+1),((obfuscate==1) && ((level>0) || $obfuscate_exclude.nil? || !$obfuscate_exclude.include?(filename)) ? 1 : 0))
    else
      if (obfuscate==1) && (((!$obfuscate_js.nil?) && File.extname(filename).eql?(".js")) || ((!$obfuscate_css.nil?) && File.extname(filename).eql?(".css")))
        puts Jake.run('java',['-jar', $obfuscator, filepath, '-o', dst_path])
        unless $? == 0
          puts "Obfuscation error"
          exit 1
        end
      else
        cp filepath, dst_path, :preserve => true
      end
    end
  end
end

def common_bundle_start(startdir, dest)
  puts "common_bundle_start"
  
  app = $app_path
  rhodeslib = "lib/framework"

  rm_rf $srcdir
  mkdir_p $srcdir
  mkdir_p dest if not File.exists? dest
  mkdir_p File.join($srcdir,'apps')

  start = pwd
  chdir rhodeslib

  Dir.glob("*").each { |f| cp_r f,dest, :preserve => true }

  chdir dest
  Dir.glob("**/rhodes-framework.rb").each {|f| rm f}
  Dir.glob("**/erb.rb").each {|f| rm f}
  Dir.glob("**/find.rb").each {|f| rm f}
  $excludelib.each {|e| Dir.glob(e).each {|f| rm f}}

  chdir start
  clear_linker_settings

  init_extensions(startdir, dest)

  chdir startdir
  #throw "ME"
  cp_r app + '/app',File.join($srcdir,'apps'), :preserve => true
  if File.exists? app + '/public'
    if $obfuscate_js.nil? && $obfuscate_css.nil?
      cp_r app + '/public', File.join($srcdir,'apps'), :preserve => true 
    else
      public_folder_cp_r app + '/public', File.join($srcdir,'apps/public'), 0, 1
    end
  end
  cp app + '/rhoconfig.txt', File.join($srcdir,'apps'), :preserve => true

  app_version = "\r\napp_version='#{$app_config["version"]}'"  
  File.open(File.join($srcdir,'apps/rhoconfig.txt'), "a"){ |f| f.write(app_version) }
  File.open(File.join($srcdir,'apps/rhoconfig.txt.timestamp'), "w"){ |f| f.write(Time.now.to_f().to_s()) }
  
  unless $debug
    rm_rf $srcdir + "/apps/app/test"
    rm_rf $srcdir + "/apps/app/SpecRunner"
    rm_rf $srcdir + "/apps/app/mspec"
    rm_rf $srcdir + "/apps/app/mspec.rb"
    rm_rf $srcdir + "/apps/app/spec_runner.rb"
  end


  copy_assets($assetfolder) if ($assetfolder and File.exists? $assetfolder)

  replace_platform = $config['platform']
  replace_platform = "bb6" if $bb6
  replace_platform = "wm" if replace_platform == 'win32'

  [File.join($srcdir,'apps'), ($current_platform == "bb" ? File.join($srcdir,'res') : File.join($srcdir,'lib/res'))].each do |folder|
      chdir folder
      
      Dir.glob("**/*.#{replace_platform}.*").each do |file|
        oldfile = file.gsub(Regexp.new(Regexp.escape('.') + replace_platform + Regexp.escape('.')),'.')
        rm oldfile if File.exists? oldfile
        mv file,oldfile
      end
      
      Dir.glob("**/*.wm.*").each { |f| rm f }
	    Dir.glob("**/*.wp7.*").each { |f| rm f }
      Dir.glob("**/*.iphone.*").each { |f| rm f }
      Dir.glob("**/*.bb.*").each { |f| rm f }
      Dir.glob("**/*.bb6.*").each { |f| rm f }
      Dir.glob("**/*.android.*").each { |f| rm f }
      Dir.glob("**/.svn").each { |f| rm_rf f }
      Dir.glob("**/CVS").each { |f| rm_rf f }
  end  
end

def create_manifest
    require File.dirname(__FILE__) + '/lib/framework/rhoappmanifest'
    
    fappManifest = Rho::AppManifest.enumerate_models(File.join($srcdir, 'apps/app'))
    content = fappManifest.read();
    
    File.open( File.join($srcdir,'apps/app_manifest.txt'), "w"){|file| file.write(content)}    
end

def process_exclude_folders
  excl = []

  exclude_platform = $config['platform']
  exclude_platform = "bb6" if $bb6
  exclude_platform = "wm" if exclude_platform == 'win32'

  if $app_config["excludedirs"]
      excl << $app_config["excludedirs"]['all'] if $app_config["excludedirs"]['all']
      excl << $app_config["excludedirs"][exclude_platform] if $app_config["excludedirs"][exclude_platform]
  end
      
  if  $config["excludedirs"]    
      excl << $config["excludedirs"]['all'] if $config["excludedirs"]['all']
      excl << $config["excludedirs"][exclude_platform] if $config["excludedirs"][exclude_platform]
  end  
  
  if excl.size() > 0
      chdir File.join($srcdir)#, 'apps')
  
      excl.each do |mask|
        Dir.glob(mask).each {|f| puts "f: #{f}"; rm_rf f}
      end
  end

end
  
namespace "build" do
  namespace "bundle" do
    task :xruby do
      #needs $config, $srcdir, $excludelib, $bindir
      app = $app_path
  	  jpath = $config["env"]["paths"]["java"]
      startdir = pwd
      dest =  $srcdir
      xruby =  File.dirname(__FILE__) + '/res/build-tools/xruby-0.3.3.jar'
      compileERB = "lib/build/compileERB/bb.rb"
      rhodeslib = File.dirname(__FILE__) + "/lib/framework"
      
      common_bundle_start(startdir,dest)

      process_exclude_folders()
      
      cp_r File.join(startdir, "platform/shared/db/res/db"), File.join($srcdir, 'apps')
      
      chdir startdir
      
      #create manifest
      create_manifest
      
      #"compile ERB"
      #ext = ".erb"
      #Find.find($srcdir) do |path|
      #  if File.extname(path) == ext
      #    rbText = ERB.new( IO.read(path) ).src
      #    newName = File.basename(path).sub('.erb','_erb.rb')
      #    fName = File.join(File.dirname(path), newName)
      #    frb = File.new(fName, "w")
      #    frb.write( rbText )
      #    frb.close()
      #  end
      #end
      cp   compileERB, $srcdir
      puts "Running bb.rb"

      puts `#{$rubypath} -I#{rhodeslib} "#{$srcdir}/bb.rb"`
      unless $? == 0
        puts "Error interpreting erb code"
        exit 1
      end

      rm "#{$srcdir}/bb.rb"

      chdir $bindir
      # -n#{$bundleClassName}
      output = `java -jar "#{xruby}" -v -c RhoBundle 2>&1`
      output.each_line { |x| puts ">>> " + x  }
      unless $? == 0
        puts "Error interpreting ruby code"
        exit 1
      end
      chdir startdir
      chdir $srcdir
  
      Dir.glob("**/*.rb") { |f| rm f }
      Dir.glob("**/*.erb") { |f| rm f }
=begin
      # RubyIDContainer.* files takes half space of jar why we need it?
      Jake.unjar("../RhoBundle.jar", $tmpdir)
      Dir.glob($tmpdir + "/**/RubyIDContainer.class") { |f| rm f }
      rm "#{$bindir}/RhoBundle.jar"
      chdir $tmpdir
      puts `jar cf #{$bindir}/RhoBundle.jar #{$all_files_mask}`      
      rm_rf $tmpdir
      mkdir_p $tmpdir
      chdir $srcdir
=end  

      puts `"#{File.join(jpath,'jar')}" uf ../RhoBundle.jar apps/#{$all_files_mask}`
      unless $? == 0
        puts "Error creating Rhobundle.jar"
        exit 1
      end
      chdir startdir
      
    end

    task :noxruby do
      app = $app_path
      rhodeslib = File.dirname(__FILE__) + "/lib/framework"
      compileERB = "lib/build/compileERB/default.rb"
      compileRB = "lib/build/compileRB/compileRB.rb"
      startdir = pwd
      dest = $srcdir + "/lib"      

      common_bundle_start(startdir,dest)
      process_exclude_folders
      chdir startdir
      
      create_manifest
      
      cp   compileERB, $srcdir
      puts "Running default.rb"

      puts `#{$rubypath} -I#{rhodeslib} "#{$srcdir}/default.rb"`
      unless $? == 0
        puts "Error interpreting erb code"
        exit 1
      end

      rm "#{$srcdir}/default.rb"

      cp   compileRB, $srcdir
      puts "Running compileRB"
      puts `#{$rubypath} -I#{rhodeslib} "#{$srcdir}/compileRB.rb"`
      unless $? == 0
        puts "Error interpreting ruby code"
        exit 1
      end

      chdir $srcdir
      Dir.glob("**/*.rb") { |f| rm f }
      Dir.glob("**/*.erb") { |f| rm f }
  
      chdir startdir

      cp_r "platform/shared/db/res/db", $srcdir 
    end
    
    task :upgrade_package do
    
      $bindir = File.join($app_path, "bin") 
      $current_platform = 'empty'
      $srcdir = File.join($bindir, "RhoBundle")
    
      $targetdir = File.join($bindir, "target")
      $excludelib = ['**/builtinME.rb','**/ServeME.rb','**/dateME.rb','**/rationalME.rb']
      $tmpdir = File.join($bindir, "tmp")
      $appname = $app_config["name"]
      $appname = "Rhodes" if $appname.nil?
      $vendor = $app_config["vendor"]
      $vendor = "rhomobile" if $vendor.nil?
      $vendor = $vendor.gsub(/^[^A-Za-z]/, '_').gsub(/[^A-Za-z0-9]/, '_').gsub(/_+/, '_').downcase
      $appincdir = File.join $tmpdir, "include"

      Rake::Task["config:common"].invoke

      Rake::Task["build:bundle:noxruby"].invoke
      
      new_zip_file = File.join($srcdir, "apps", "upgrade_bundle.zip")
      
      if RUBY_PLATFORM =~ /(win|w)32$/
        begin
      
          require 'rubygems'
          require 'zip/zip'
          require 'find'
          require 'fileutils'
          include FileUtils

          root = $srcdir
          
          new_zip_file = File.join($srcdir, "upgrade_bundle.zip")

          Zip::ZipFile.open(new_zip_file, Zip::ZipFile::CREATE)do |zipfile|
            Find.find(root) do |path|
                  Find.prune if File.basename(path)[0] == ?.
                  dest = /apps\/(\w.*)/.match(path)
                  if dest
                      puts '     add file to zip : '+dest[1].to_s
                      zipfile.add(dest[1],path)
                  end
            end 
          end
        rescue
          puts 'ERROR !'
          puts 'Require "rubyzip" gem for make zip file !'
          puts 'Install gem by "gem install rubyzip"'
        end        
      else
        chdir File.join($srcdir, "apps")
        sh %{zip -r upgrade_bundle.zip .}
      end

      cp   new_zip_file, $bindir
      
      rm   new_zip_file
      
    end
    

    task :noiseq do
      app = $app_path
      rhodeslib = File.dirname(__FILE__) + "/lib/framework"
	    compileERB = "lib/build/compileERB/bb.rb"
      startdir = pwd
      dest = $srcdir + "/lib"      

      common_bundle_start(startdir,dest)
      process_exclude_folders
      chdir startdir
      
      create_manifest
  
	  cp   compileERB, $srcdir
      puts "Running bb.rb"

      puts `#{$rubypath} -I#{rhodeslib} "#{$srcdir}/bb.rb"`
      unless $? == 0
        puts "Error interpreting erb code"
        exit 1
      end

      rm "#{$srcdir}/bb.rb"

      chdir $srcdir
      Dir.glob("**/*.erb") { |f| rm f }

	  chdir startdir
      cp_r "platform/shared/db/res/db", $srcdir 
    end
  end
end


# Simple rakefile that loads subdirectory 'rhodes' Rakefile
# run "rake -T" to see list of available tasks

#desc "Get versions"
task :get_version do

  #genver = "unknown"
  iphonever = "unknown"
  #symver = "unknown"
  wmver = "unknown"
  androidver = "unknown"
  

  # File.open("res/generators/templates/application/build.yml","r") do |f|
  #     file = f.read
  #     if file.match(/version: (\d+\.\d+\.\d+)/)
  #       genver = $1
  #     end
  #   end

  File.open("platform/iphone/Info.plist","r") do |f|
    file = f.read
    if file.match(/CFBundleVersion<\/key>\s+<string>(\d+\.\d+\.*\d*)<\/string>/)
      iphonever =  $1
    end
  end

  # File.open("platform/symbian/build/release.properties","r") do |f|
  #     file = f.read
  #     major = ""
  #     minor = ""
  #     build = ""
  # 
  #     if file.match(/release\.major=(\d+)/)
  #       major =  $1
  #     end
  #     if file.match(/release\.minor=(\d+)/)
  #       minor =  $1
  #     end
  #     if file.match(/build\.number=(\d+)/)
  #       build =  $1
  #     end
  # 
  #     symver = major + "." + minor + "." + build
  #   end

  File.open("platform/android/Rhodes/AndroidManifest.xml","r") do |f|
    file = f.read
    if file.match(/versionName="(\d+\.\d+\.*\d*)"/)
      androidver =  $1
    end
  end

  gemver = "unknown"
  rhodesver = "unknown"
  frameworkver = "unknown"

  File.open("lib/rhodes.rb","r") do |f|
    file = f.read
    if file.match(/VERSION = '(\d+\.\d+\.*\d*)'/)
      gemver =  $1
    end
  end

  File.open("lib/framework/rhodes.rb","r") do |f|
    file = f.read
    if file.match(/VERSION = '(\d+\.\d+\.*\d*)'/)
      rhodesver =  $1
    end
  end

  File.open("lib/framework/version.rb","r") do |f|
    file = f.read
    if file.match(/VERSION = '(\d+\.\d+\.*\d*)'/)
      frameworkver =  $1
    end
  end

  

  puts "Versions:"
  #puts "  Generator:        " + genver
  puts "  iPhone:           " + iphonever
  #puts "  Symbian:          " + symver
  #puts "  WinMo:            " + wmver
  puts "  Android:          " + androidver
  puts "  Gem:              " + gemver
  puts "  Rhodes:           " + rhodesver
  puts "  Framework:        " + frameworkver
end

#desc "Set version"
task :set_version, [:version] do |t,args|
  throw "You must pass in version" if args.version.nil?
  ver = args.version.split(/\./)
  major = ver[0]
  minor = ver[1]
  build = ver[2]
  
  throw "Invalid version format. Must be in the format of: major.minor.build" if major.nil? or minor.nil? or build.nil?

  verstring = major+"."+minor+"."+build
  origfile = ""

  # File.open("res/generators/templates/application/build.yml","r") { |f| origfile = f.read }
  #   File.open("res/generators/templates/application/build.yml","w") do |f|
  #     f.write origfile.gsub(/version: (\d+\.\d+\.\d+)/, "version: #{verstring}")
  #   end
  

  File.open("platform/iphone/Info.plist","r") { |f| origfile = f.read }
  File.open("platform/iphone/Info.plist","w") do |f| 
    f.write origfile.gsub(/CFBundleVersion<\/key>(\s+)<string>(\d+\.\d+\.*\d*)<\/string>/, "CFBundleVersion</key>\n\t<string>#{verstring}</string>")
  end

  # File.open("platform/symbian/build/release.properties","r") { |f| origfile = f.read }
  # File.open("platform/symbian/build/release.properties","w") do |f|
  #   origfile.gsub!(/release\.major=(\d+)/,"release.major=#{major}")
  #   origfile.gsub!(/release\.minor=(\d+)/,"release.minor=#{minor}")
  #   origfile.gsub!(/build\.number=(\d+)/,"build.number=#{build}")
  #   f.write origfile
  # end

  File.open("platform/android/Rhodes/AndroidManifest.xml","r") { |f| origfile = f.read }
  File.open("platform/android/Rhodes/AndroidManifest.xml","w") do |f|
    origfile.match(/versionCode="(\d+)"/)
    vercode = $1.to_i + 1
    origfile.gsub!(/versionCode="(\d+)"/,"versionCode=\"#{vercode}\"")
    origfile.gsub!(/versionName="(\d+\.\d+\.*\d*)"/,"versionName=\"#{verstring}\"")

    f.write origfile
  end
  ["lib/rhodes.rb","lib/framework/rhodes.rb","lib/framework/version.rb"].each do |versionfile|
  
    File.open(versionfile,"r") { |f| origfile = f.read }
    File.open(versionfile,"w") do |f|
      origfile.gsub!(/^(\s*VERSION) = '(\d+\.\d+\.*\d*)'/, '\1 = \''+ verstring + "'")     
      f.write origfile
    end
  end

  Rake::Task[:get_version].invoke  
end



namespace "buildall" do
  namespace "bb" do
    #    desc "Build all jdk versions for blackberry"
    task :production => "config:common" do
      $config["env"]["paths"].each do |k,v|
        if k.to_s =~ /^4/
          puts "BUILDING VERSION: #{k}"
          $app_config["bbver"] = k
#          Jake.reconfig($config)
 
          #reset all tasks used for building
          Rake::Task["config:bb"].reenable
          Rake::Task["build:bb:rhobundle"].reenable
          Rake::Task["build:bb:rhodes"].reenable
          Rake::Task["build:bb:rubyvm"].reenable
          Rake::Task["device:bb:dev"].reenable
          Rake::Task["device:bb:production"].reenable
          Rake::Task["device:bb:rhobundle"].reenable
          Rake::Task["package:bb:dev"].reenable
          Rake::Task["package:bb:production"].reenable
          Rake::Task["package:bb:rhobundle"].reenable
          Rake::Task["package:bb:rhodes"].reenable
          Rake::Task["package:bb:rubyvm"].reenable
          Rake::Task["device:bb:production"].reenable
          Rake::Task["clean:bb:preverified"].reenable

          Rake::Task["clean:bb:preverified"].invoke
          Rake::Task["device:bb:production"].invoke
        end
      end

    end
  end
end


task :gem do
  puts "Removing old gem"
  rm_rf Dir.glob("rhodes*.gem")
  puts "Copying Rakefile"
  cp "Rakefile", "rakefile.rb"
  
  puts "Building manifest"
  out = ""
  Dir.glob("**/*") do |fname| 
    # TODO: create exclusion list
    out << fname + "\n" if File.file? fname and not fname =~ /rhoconnect-client/
  end
  File.open("Manifest.txt",'w') {|f| f.write(out)}

  puts "Loading gemspec"
  require 'rubygems'
  spec = Gem::Specification.load('rhodes.gemspec')

  puts "Building gem"
  gemfile = Gem::Builder.new(spec).build
end

namespace "rhomobile-debug" do
    task :gem do
      puts "Removing old gem"
      rm_rf Dir.glob("rhomobile-debug*.gem")
      rm_rf "rhomobile-debug"
      
      mkdir_p "rhomobile-debug"
      mkdir_p "rhomobile-debug/lib"
      cp 'lib/extensions/debugger/debugger.rb', "rhomobile-debug/lib", :preserve => true
      cp 'lib/extensions/debugger/README.md', "rhomobile-debug", :preserve => true
      cp 'lib/extensions/debugger/LICENSE', "rhomobile-debug", :preserve => true
      cp 'lib/extensions/debugger/CHANGELOG', "rhomobile-debug", :preserve => true
      
      cp 'rhomobile-debug.gemspec', "rhomobile-debug", :preserve => true
      
      startdir = pwd
      chdir 'rhomobile-debug'
      
      puts "Loading gemspec"
      require 'rubygems'
      spec = Gem::Specification.load('rhomobile-debug.gemspec')

      puts "Building gem"
      gemfile = Gem::Builder.new(spec).build
      
      Dir.glob("rhomobile-debug*.gem").each do |f|
        cp f, startdir, :preserve => true      
      end

      chdir startdir
      rm_rf "rhomobile-debug"
    end
end

task :tasks do
  Rake::Task.tasks.each {|t| puts t.to_s.ljust(27) + "# " + t.comment.to_s}
end

task :switch_app => "config:common" do
  rhobuildyml = File.dirname(__FILE__) + "/rhobuild.yml"
  if File.exists? rhobuildyml
    config = YAML::load_file(rhobuildyml)
  else
    puts "Cant find rhobuild.yml"
    exit 1
  end
  config["env"]["app"] = $app_path.gsub(/\\/,"/")
  File.open(  rhobuildyml, 'w' ) do |out|
    YAML.dump( config, out )
  end
end


#Rake::RDocTask.new do |rd|
#RDoc::Task.new do |rd|
#  rd.main = "README.textile"
#  rd.rdoc_files.include("README.textile", "lib/framework/**/*.rb")
#end
#Rake::Task["rdoc"].comment=nil
#Rake::Task["rerdoc"].comment=nil

#task :rdocpush => :rdoc do
#  puts "Pushing RDOC. This may take a while"
#  `scp -r html/* dev@dev.rhomobile.com:dev.rhomobile.com/rhodes/`
#end

namespace "build" do
    #    desc "Build rhoconnect-client package"
    task :rhoconnect_client do

        ver = File.read("rhoconnect-client/version").chomp #.gsub(".", "_")
        zip_name = "rhoconnect-client-"+ver+".zip"

        bin_dir = "rhoconnect-client-bin"
        src_dir = bin_dir + "/rhoconnect-client-"+ver #"/src"        
        shared_dir = src_dir + "/platform/shared"        
        rm_rf bin_dir
        rm    zip_name if File.exists? zip_name
        mkdir_p bin_dir
        mkdir_p src_dir

        cp_r 'rhoconnect-client', src_dir, :preserve => true
        
        mv src_dir+"/rhoconnect-client/license", src_dir
        mv src_dir+"/rhoconnect-client/README.textile", src_dir
        mv src_dir+"/rhoconnect-client/version", src_dir
        mv src_dir+"/rhoconnect-client/changelog", src_dir
                
        Dir.glob(src_dir+"/rhoconnect-client/**/*").each do |f|
		    #puts f
		    
            rm_rf f if f.index("/build/") || f.index(".DS_Store")         
 
        end
		
        mkdir_p shared_dir
        
        Dir.glob("platform/shared/*").each do |f|
            next if f == "platform/shared/ruby" || f == "platform/shared/rubyext" || f == "platform/shared/xruby" || f == "platform/shared/shttpd" ||
                f == "platform/shared/stlport"  || f == "platform/shared/qt"
            #puts f                
            cp_r f, shared_dir #, :preserve => true                        
        end
        startdir = pwd
        chdir bin_dir
        puts `zip -r #{File.join(startdir, zip_name)} *`
                
        chdir startdir
        
        rm_rf bin_dir
    end
end

namespace "run" do

    desc "Run application on RhoSimulator"
    task :rhosimulator_base => "config:common" do
        puts "rho_reload_app_changes : #{ENV['rho_reload_app_changes']}"
        $path = ""
        $args = ["-approot='#{$app_path}'", "-rhodespath='#{$startdir}'"]
        cmd = nil

        if RUBY_PLATFORM =~ /(win|w)32$/
            if $config['env']['paths']['rhosimulator'] and $config['env']['paths']['rhosimulator'].length() > 0
                $path = File.join( $config['env']['paths']['rhosimulator'], "rhosimulator.exe" )
            else
                $path = File.join( $startdir, "platform/win32/RhoSimulator/rhosimulator.exe" )
            end
        elsif RUBY_PLATFORM =~ /darwin/
            if $config['env']['paths']['rhosimulator'] and $config['env']['paths']['rhosimulator'].length() > 0
                $path = File.join( $config['env']['paths']['rhosimulator'], "RhoSimulator.app" )
            else
                $path = File.join( $startdir, "platform/osx/bin/RhoSimulator/RhoSimulator.app" )
            end
            cmd = 'open'
            $args.unshift($path, '--args')
        else
            if $config['env']['paths']['rhosimulator'] and $config['env']['paths']['rhosimulator'].length() > 0
                # $path = File.join( $config['env']['paths']['rhosimulator'], "RhoSimulator" )
            else
                # $path = File.join( $startdir, "platform/linux/bin/RhoSimulator/RhoSimulator" )
            end
            $args << ">/dev/null"
            $args << "2>/dev/null"
        end

        $appname = $app_config["name"].nil? ? "Rhodes" : $app_config["name"]
        if !File.exists?($path)
            puts "Cannot find RhoSimulator: '#{$path}' does not exists"
            puts "Check sdk path in build.yml - it should point to latest rhodes (run 'set-rhodes-sdk' in application folder) OR"

            if $config['env']['paths']['rhosimulator'] and $config['env']['paths']['rhosimulator'].length() > 0
                puts "Check 'env:paths:rhosimulator' path in '<rhodes>/rhobuild.yml' OR"
            end
            
            puts "Install Rhodes gem OR"
            puts "Install RhoSimulator and modify 'env:paths:rhosimulator' section in '<rhodes>/rhobuild.yml'"
            exit 1
        end

        sim_conf = "rhodes_path='#{$startdir}'\r\n"
        sim_conf += "app_version='#{$app_config["version"]}'\r\n"
        sim_conf += "app_name='#{$appname}'\r\n"
        if ( ENV['rho_reload_app_changes'] )
            sim_conf += "reload_app_changes=#{ENV['rho_reload_app_changes']}\r\n"        
        else
            sim_conf += "reload_app_changes=1\r\n"                    
        end

        if $config['debug']
            sim_conf += "debug_port=#{$config['debug']['port']}\r\n"
        else
            sim_conf += "debug_port=\r\n"
        end    
        
        if $config['debug'] && $config['debug']['host'] && $config['debug']['host'].length() > 0
            sim_conf += "debug_host='#{$config['debug']['host']}'\r\n"        
        else    
            sim_conf += "debug_host='127.0.0.1'\r\n"
        end
        
        sim_conf += $rhosim_config if $rhosim_config

        #check gem extensions
        config_ext_paths = ""
        extpaths = $app_config["extpaths"]        
        $app_config["extensions"].each do |extname|
        
            extpath = nil
            extpaths.each do |p|
             if p
              ep = File.join(p, extname)
              if File.exists? ep
                extpath = ep
                break
              end
              end
            end

            if extpath.nil?
                begin
                    $rhodes_extensions = nil
                    require extname
                    extpath = $rhodes_extensions[0] unless $rhodes_extensions.nil?
                    config_ext_paths += "#{extpath};" if extpath && extpath.length() > 0 
                rescue Exception => e
                end
            else
            
                if $config["platform"] != "bb"
                    extyml = File.join(extpath, "ext.yml")
                    next if File.file? extyml
                end
                
                config_ext_paths += "#{extpath};" if extpath && extpath.length() > 0                     
            end    
        end

        sim_conf += "ext_path=#{config_ext_paths}\r\n" if config_ext_paths && config_ext_paths.length() > 0 
        
        fdir = File.join($app_path, 'rhosimulator')
        mkdir fdir unless File.exist?(fdir)
            
        fname = File.join(fdir, 'rhosimconfig.txt')
        File.open(fname, "wb") do |fconf|
            fconf.write( sim_conf )
        end

        if not cmd.nil?
            $path = cmd
        end
    end

    task :rhosimulator => "run:rhosimulator_base" do
        puts 'start rhosimulator'
        Jake.run2 $path, $args, {:nowait => true}
    end

    task :rhosimulator_debug => "run:rhosimulator_base" do
        puts 'start rhosimulator debug'
        Jake.run2 $path, $args, {:nowait => true}

        if RUBY_PLATFORM =~ /darwin/
	  while 1
          end     
        end
    end

end

namespace "build" do
    task :rhosimulator => "config:common" do
        $rhodes_version = File.read(File.join($startdir,'version')).chomp
        File.open(File.join($startdir, 'platform/shared/qt/rhodes/RhoSimulatorVersion.h'), "wb") do |fversion|
            fversion.write( "#define RHOSIMULATOR_VERSION \"#{$rhodes_version}\"\n" )
        end
        if RUBY_PLATFORM =~ /(win|w)32$/
            Rake::Task["build:win32:rhosimulator"].invoke
        elsif RUBY_PLATFORM =~ /darwin/
            Rake::Task["build:osx:rhosimulator"].invoke
        else
            puts "Sorry, at this time RhoSimulator can be built for Windows and Mac OS X only"
            exit 1
        end
    end
end
