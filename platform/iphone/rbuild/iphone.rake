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


def extract_value_from_strings(line)
   pre_str = '<string>'
   post_str = '</string>'
   pre_index = line.index(pre_str)
   post_index = line.index(post_str)
   return line.slice( pre_index + pre_str.length, post_index - (pre_index + pre_str.length))
end

def set_app_name(newname)
  ret_value = ''
  fname = $config["build"]["iphonepath"] + "/Info.plist"
  nextline = false
  replaced = false
  buf = ""
  File.new(fname,"r").read.each_line do |line|
    if nextline and not replaced
      ret_value = extract_value_from_strings(line)
      return ret_value if line =~ /#{newname}/
      buf << line.gsub(/<string>.*<\/string>/,"<string>#{newname}</string>")
      puts "set name"
      replaced = true
    else
      buf << line
    end
    nextline = true if line =~ /CFBundleDisplayName/
  end
  File.open(fname,"w") { |f| f.write(buf) }
  return ret_value
end

def set_app_version(newversion)
  ret_value = ''
  fname = $config["build"]["iphonepath"] + "/Info.plist"
  nextline = false
  replaced = false
  buf = ""
  File.new(fname,"r").read.each_line do |line|
    if nextline and not replaced
      ret_value = extract_value_from_strings(line)
      return if line =~ /#{newversion}/
      buf << line.gsub(/<string>.*<\/string>/,"<string>#{newversion}</string>")
      puts "set name"
      replaced = true
    else
      buf << line
    end
    nextline = true if line =~ /CFBundleVersion/
  end
  File.open(fname,"w") { |f| f.write(buf) }
  return ret_value
end


def set_app_bundle_identifier(newname)
  name_cleared = newname.downcase.split(/[^a-zA-Z0-9\.\-]/).map{|w| w.downcase}.join("")
  ret_value = ''
  fname = $config["build"]["iphonepath"] + "/Info.plist"
  nextline = false
  replaced = false
  buf = ""
  File.new(fname,"r").read.each_line do |line|
    if nextline and not replaced
      ret_value = extract_value_from_strings(line)
      return ret_value if line =~ /#{name_cleared}/
      buf << line.gsub(/<string>.*<\/string>/,"<string>#{name_cleared}</string>")
      puts "set bundle identifier"
      replaced = true
    else
      buf << line
    end
    nextline = true if line =~ /CFBundleIdentifier/
  end
  File.open(fname,"w") { |f| f.write(buf) }
  return ret_value
end

def set_app_url_scheme(newname)
  name_cleared = newname.downcase.split(/[^a-zA-Z0-9\.\-]/).map{|w| w.downcase}.join("")
  ret_value = ''
  fname = $config["build"]["iphonepath"] + "/Info.plist"
  nextline = false
  nextnextline = false
  replaced = false
  buf = ""
  File.new(fname,"r").read.each_line do |line|
    if nextline and not replaced
      ret_value = extract_value_from_strings(line)
      return ret_value if line =~ /#{name_cleared}/
      buf << line.gsub(/<string>.*<\/string>/,"<string>#{name_cleared}</string>")
      puts "set URL Scheme"
      replaced = true
    else
      if nextnextline
          nextline = true
      end
      buf << line
    end
    nextnextline = true if line =~ /CFBundleURLSchemes/
  end
  File.open(fname,"w") { |f| f.write(buf) }
  return ret_value
end

BAKUP_FILES = ['rhorunner.xcodeproj', 'Entitlements.plist', 'icon.png', 'icon114.png', 'icon57.png', 'icon72.png', 'Info.plist']
CLEAR_FILES = ['Default.png', 'Default@2x.png', 'Default-Portrait.png', 'Default-PortraitUpsideDown.png', 'Default-Landscape.png', 'Default-LadscapeLeft.png', 'Default-LandscapeRight.png']

def make_project_bakup
     BAKUP_FILES.each do |f|
           filename_origin = $config["build"]["iphonepath"] + "/" +f
           filename_bak = $config["build"]["iphonepath"] + "/project_bakup/" +f
           is_folder = File.directory? filename_bak 
           if File.exists? filename_origin
                if (!File.exists? filename_bak) && (!is_folder) 
                    bak_folder = $config["build"]["iphonepath"] + "/project_bakup"
                    mkdir_p bak_folder
                    cp_r filename_origin,filename_bak
                end
           end        
     end
end

def restore_project_from_bak
     BAKUP_FILES.each do |f|
           filename_origin = $config["build"]["iphonepath"] + "/" +f
           filename_bak = $config["build"]["iphonepath"] + "/project_bakup/" +f
           if File.exists? filename_bak
                   rm_rf filename_origin 
                   cp_r filename_bak,filename_origin                      
           end        
     end
     CLEAR_FILES.each do |f|
           filename = $config["build"]["iphonepath"] + "/" +f
           if File.exists? filename
                   rm_rf filename                      
           end        
     end
end

def set_app_url_name(newname)
  ret_value = ''
  fname = $config["build"]["iphonepath"] + "/Info.plist"
  nextline = false
  replaced = false
  buf = ""
  File.new(fname,"r").read.each_line do |line|
    if nextline and not replaced
      ret_value = extract_value_from_strings(line)
      return ret_value if line =~ /#{newname}/
      buf << line.gsub(/<string>.*<\/string>/,"<string>#{newname}</string>")
      puts "set URL name"
      replaced = true
    else
      buf << line
    end
    nextline = true if line =~ /CFBundleURLName/
  end
  File.open(fname,"w") { |f| f.write(buf) }
  return ret_value
end

def make_app_info
  fname = File.join($app_path, 'bin', 'target', 'iOS', $sdk, $configuration, 'app_info.txt')
  buf = ""
  urlscheme = 'rhodes'
  urlscheme = $app_config["name"] unless $app_config["name"].nil?
  urlscheme = $app_config["iphone"]["BundleURLScheme"] unless $app_config["iphone"]["BundleURLScheme"].nil?
  buf << urlscheme
  File.open(fname,"w") { |f| f.write(buf) }
end


def prepare_production_ipa (app_path, app_name)
  puts 'Preparing *.IPA file ...'

  tmp_dir = File.join(app_path, "tmp_ipa")
  mkdir_p tmp_dir
  payload_dir = File.join(tmp_dir, "Payload")  
  mkdir_p payload_dir
  app_file = File.join(app_path, app_name + ".app")
  app_in_payload = File.join(payload_dir,  app_name + ".app")

  mprovision_in_app = File.join(app_file,  "embedded.mobileprovision") 
  mprovision_in_payload = File.join(payload_dir,  app_name + ".mobileprovision") 
  
  cp_r app_file, app_in_payload
  
  cp mprovision_in_app, mprovision_in_payload
  
  
  itunes_artwork = File.join($config["build"]["iphonepath"], "iTunesArtwork.jpg")
  itunes_artwork_dst = File.join(tmp_dir, "iTunesArtwork")
  
  if !$app_config["iphone"].nil?
    if !$app_config["iphone"]["production"].nil?
      if !$app_config["iphone"]["production"]["ipa_itunesartwork_image"].nil?
        art_test_name = $app_config["iphone"]["production"]["ipa_itunesartwork_image"]
        if File.exists? art_test_name
          itunes_artwork = art_test_name
        else
          art_test_name = File.join($app_path,$app_config["iphone"]["production"]["ipa_itunesartwork_image"])
          if File.exists? art_test_name
            itunes_artwork = art_test_name
          else 
            itunes_artwork = $app_config["iphone"]["production"]["ipa_itunesartwork_image"]  
          end  
        end 
      end
    end
  end
  
  cp itunes_artwork, itunes_artwork_dst
  
  currentdir = Dir.pwd()      
  chdir tmp_dir 
  sh %{zip -r temporary_archive.zip .}
  ipa_file_path = File.join(app_path, app_name + ".ipa")  
  rm_rf ipa_file_path 
  cp 'temporary_archive.zip', ipa_file_path     
  Dir.chdir currentdir         
  rm_rf tmp_dir

end

def copy_all_png_from_icon_folder_to_product(app_path)
   app_icon_folder = File.join($app_path, 'icon')
   rm_rf File.join(app_path, "*.png")
   Dir.glob(File.join(app_icon_folder, "*.png")).each do |icon_file|
     cp icon_file, app_path
   end
end

def prepare_production_plist (app_path, app_name)
    puts 'Preparing application production plist ...'
  
    $plist_title = app_name
    $plist_subtitle = app_name
    $plist_icon_url = "http://example.com/icon57.png"
    $plist_ipa_url = "http://example.com/"+app_name+".ipa"
    
    appname = $app_config["name"] ? $app_config["name"] : "rhorunner"
    vendor = $app_config['vendor'] ? $app_config['vendor'] : "rhomobile"
    $plist_bundle_id = "com.#{vendor}.#{appname}"
    $plist_bundle_id = $app_config["iphone"]["BundleIdentifier"] unless $app_config["iphone"]["BundleIdentifier"].nil?
    
    $plist_bundle_version = "1.0"
    $plist_bundle_version = $app_config["version"] unless $app_config["version"].nil?

    if !$app_config["iphone"].nil?
      if !$app_config["iphone"]["production"].nil?
        $plist_title = $app_config["iphone"]["production"]["app_plist_title"] unless $app_config["iphone"]["production"]["app_plist_title"].nil? 
        $plist_subtitle = $app_config["iphone"]["production"]["app_plist_subtitle"] unless $app_config["iphone"]["production"]["app_plist_subtitle"].nil? 
        $plist_icon_url = $app_config["iphone"]["production"]["app_plist_icon_url"] unless $app_config["iphone"]["production"]["app_plist_icon_url"].nil? 
        $plist_ipa_url = $app_config["iphone"]["production"]["app_plist_ipa_url"] unless $app_config["iphone"]["production"]["app_plist_ipa_url"].nil? 
      end
    end        
  
    rbText = ERB.new( IO.read(File.join($config["build"]["iphonepath"], "rbuild", "ApplicationPlist.erb")) ).result
	fAlx = File.new(File.join(app_path, app_name + ".plist"), "w")
    fAlx.write(rbText)
    fAlx.close()
end

ICONS = [['icon', 'icon57'], ['icon57','icon57'], ['icon72','icon72'], ['icon114','icon114']]

def copy_all_icons_to_production_folder
  ICONS.each do |pair|

  end
end

def restore_app_icon
  puts "restore icon"
  ipath = $config["build"]["iphonepath"]
  ICONS.each do |pair|
    name = pair[0]
    ibak = File.join(ipath, name + '.bak')
    icon = File.join(ipath, name + '.png')
    next if !File.exists? ibak
    rm_f icon
    cp ibak, icon
    rm_f ibak
  end
end

def set_app_icon(make_bak)
  puts "set icon"
  ipath = $config["build"]["iphonepath"]
  begin
    ICONS.each do |pair|
      name = pair[0]
      ibak = File.join(ipath, name + '.bak')
      icon = File.join(ipath, name + '.png')
      appicon = File.join($app_path, 'icon', pair[1] + '.png')
      if make_bak 
         cp icon, ibak unless File.exists? ibak
      end
      cp appicon, ipath
    end
  rescue => e
    puts "WARNING!!! Can not change icon: #{e.to_s}"
  end
end

LOADINGIMAGES = ['loading', 'loading@2x', 'loading-Portrait', 'loading-PortraitUpsideDown', 'loading-Landscape', 'loading-LadscapeLeft', 'loading-LandscapeRight']

def restore_default_images
  puts "restore_default_images"
  ipath = $config["build"]["iphonepath"]
  LOADINGIMAGES.each do |name|
    defname = name.sub('loading', 'Default')
    ibak = File.join(ipath, defname + '.bak')
    imag = File.join(ipath, defname + '.png')
    rm_f imag
    next if !File.exists? ibak
    rm_f imag
    cp ibak, imag
    rm_f ibak
  end
end

def set_default_images(make_bak)
  puts "set_default_images"
  ipath = $config["build"]["iphonepath"]
  begin
    LOADINGIMAGES.each do |name|
      defname = name.sub('loading', 'Default')
      ibak = File.join(ipath, defname + '.bak')
      imag = File.join(ipath, defname + '.png')
      appimage = File.join($app_path, 'app', name + '.png')
      appsimage = File.join($app_path, 'app', name + '.iphone.png')
      if File.exists? imag
        if make_bak
           cp imag, ibak unless File.exists? ibak
        end
        rm_f imag
      end
      #bundlei = File.join($srcdir, defname + '.png')
      #cp appimage, bundlei unless !File.exist? appimage
      if File.exists? appsimage
          cp appsimage, imag
      else
          cp appimage, imag unless !File.exists? appimage
      end
    end
  rescue => e
    puts "WARNING!!! Can not change default image: #{e.to_s}"
  end
end

def copy_entitlements_file_from_app
  enti_rho_name = File.join($config["build"]["iphonepath"], "Entitlements.plist")
  enti_app_name = File.join($app_path, "Entitlements.plist")

  if !$app_config["iphone"].nil?
    if !$app_config["iphone"]["entitlements_file"].nil?
      enti_test_name = $app_config["iphone"]["entitlements_file"]
      if File.exists? enti_test_name
        enti_app_name = enti_test_name
      else
        enti_test_name = File.join($app_path, $app_config["iphone"]["entitlements_file"])
        if File.exists? enti_test_name
          enti_app_name = enti_test_name
        else 
          enti_app_name = $app_config["iphone"]["entitlements_file"]  
        end  
      end 
    end  
  end

  if File.exists? enti_app_name
    puts 'Copy Entitlements.plist from application ...'
    cp enti_rho_name, (enti_rho_name + '_bak')   
    rm_f enti_rho_name
    cp enti_app_name,enti_rho_name   
  end
end

def restore_entitlements_file
  enti_rho_name = File.join($config["build"]["iphonepath"], "Entitlements.plist")
  if File.exists?(enti_rho_name + '_bak')
    puts 'restore Entitlements.plist ...'
    rm_f enti_rho_name
    cp (enti_rho_name + '_bak'), enti_rho_name   
    rm_f (enti_rho_name + '_bak')
  end  
end


def set_signing_identity(identity,profile,entitlements)
  fname = $config["build"]["iphonepath"] + "/rhorunner.xcodeproj/project.pbxproj"
  buf = ""
  File.new(fname,"r").read.each_line do |line|
      line.gsub!(/CODE_SIGN_ENTITLEMENTS = .*;/,"CODE_SIGN_ENTITLEMENTS = \"#{entitlements}\";")
      line.gsub!(/CODE_SIGN_IDENTITY = .*;/,"CODE_SIGN_IDENTITY = \"#{identity}\";")
      line.gsub!(/"CODE_SIGN_IDENTITY\[sdk=iphoneos\*\]" = .*;/,"\"CODE_SIGN_IDENTITY[sdk=iphoneos*]\" = \"#{identity}\";")
      if profile and profile.to_s != ""
        line.gsub!(/PROVISIONING_PROFILE = .*;/,"PROVISIONING_PROFILE = \"#{profile}\";")
        line.gsub!(/"PROVISIONING_PROFILE\[sdk=iphoneos\*\]" = .*;/,"\"PROVISIONING_PROFILE[sdk=iphoneos*]\" = \"#{profile}\";")
      end
      
      puts line if line =~ /CODE_SIGN/
      buf << line
  end
  
  File.open(fname,"w") { |f| f.write(buf) }
end

def basedir
  File.join(File.dirname(__FILE__),'..','..','..')
end

def app_expanded_path(appname)
  File.expand_path(File.join(basedir,'spec',appname))
end


def check_sdk(sdkname) 

      puts 'Check SDK :'
      args = ['-version', '-sdk', sdkname]

      puts Jake.run($xcodebuild,args)
      ret = $?

      chdir $startdir

      unless ret == 0
        puts ""
        puts "ERROR: invalid SDK in BUILD.YML !"
        puts sdkname+' is NOT installed on this computer !'
        puts ""

        puts "See all installed SDKs on this computer :"
        args = ['-showsdks']
        Jake.run($xcodebuild,args)
        
        exit 1
      end
end


namespace "config" do
  task :set_iphone_platform do
    $current_platform = "iphone"
  end

  task :iphone => [:set_iphone_platform, "config:common", "switch_app"] do
    $rubypath = "res/build-tools/RubyMac" #path to RubyMac
    iphonepath = $config["build"]["iphonepath"]
    $builddir = iphonepath + "/rbuild"
    $bindir = Jake.get_absolute(iphonepath) + "/bin"
    $srcdir =  $bindir + "/RhoBundle"
    $targetdir = $app_path + "/bin/target/iphone" 
    $excludelib = ['**/builtinME.rb','**/ServeME.rb','**/dateME.rb','**/rationalME.rb']
    $tmpdir =  $bindir +"/tmp"

    $devroot = '/Developer' if $devroot.nil?
    $iphonesim = File.join($startdir, 'res/build-tools/iphonesim/build/Release/iphonesim') if $iphonesim.nil?


    $xcodebuild = $devroot + "/usr/bin/xcodebuild"
    
    if !File.exists? $xcodebuild
        $xcodebuild = "/Applications/Xcode.app/Contents/Developer" + "/usr/bin/xcodebuild"
        $devroot = '/Applications/Xcode.app/Contents/Developer'
        $iphonesim = File.join($startdir, 'res/build-tools/iphonesim/build/Release/iphonesim_43')
    end
    
    if !File.exists? $xcodebuild
        puts 'ERROR: can not found XCode command line tools'
        puts 'Install XCode to default location' 
        puts 'For XCode from 4.3 and later - you should install Command Line Tools package ! Open XCode - Preferences... - Downloads - Components - Command Line Tools'
        exit 1
    end

    $homedir = ENV['HOME']
    $simdir = "#{$homedir}/Library/Application Support/iPhone Simulator/"
    $sim = $devroot + "/Platforms/iPhoneSimulator.platform/Developer/Applications"
    $guid = `uuidgen`.strip
    $applog = File.join($homedir,$app_config["applog"]) if $app_config["applog"] 

    if $app_config["iphone"].nil?
      $signidentity = $config["env"]["iphone"]["codesignidentity"]
      $provisionprofile = $config["env"]["iphone"]["provisionprofile"]
      $entitlements = $config["env"]["iphone"]["entitlements"]
      $configuration = $config["env"]["iphone"]["configuration"]
      $sdk = $config["env"]["iphone"]["sdk"]
      $emulatortarget = 'iphone'
    else
      $signidentity = $app_config["iphone"]["codesignidentity"]
      $provisionprofile = $app_config["iphone"]["provisionprofile"]
      $entitlements = $app_config["iphone"]["entitlements"]
      $configuration = $app_config["iphone"]["configuration"]
      $sdk = $app_config["iphone"]["sdk"]
      $emulatortarget = $app_config["iphone"]["emulatortarget"]
      if $emulatortarget == nil
         $emulatortarget = 'iphone'
      end
    end

    if $sdk !~ /iphone/
      if Rake.application.top_level_tasks.to_s =~ /run/
        $sdk = "iphonesimulator#{$sdk}"
      else
        $sdk = "iphoneos#{$sdk}"
      end
    end

    puts $sdk
    
    check_sdk($sdk)    

    if $sdk =~ /iphonesimulator/
      $sdkver = $sdk.gsub(/iphonesimulator/,"")
      $sdkroot = $devroot + "/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator" + $sdkver + ".sdk"
    else
      $sdkver = $sdk.gsub(/iphoneos/,"")
      $sdkroot = $devroot + "/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS" + $sdkver + ".sdk"
    end

    $emulator_version = nil
    plist = File.join($sdkroot, 'System/Library/CoreServices/SystemVersion.plist')
    if File.exists? plist
      File.open(plist, 'r') do |f|
        while line = f.gets
          next unless line =~ /<string>(#{$sdkver.gsub('.', '\.')}[^<]*)<\/string>/
          $emulator_version = $1
          break unless $emulator_version.nil?
        end
      end
    end

    unless File.exists? $homedir + "/.profile"
      File.open($homedir + "/.profile","w") {|f| f << "#" }
      chmod 0744, $homedir + "/.profile"
    end

    #if $app_config["iphone"] and $app_config["iphone"]["extensions"]
    #  $app_config["extensions"] += $app_config["iphone"]["extensions"] if $app_config["extensions"]
    #  $app_config["iphone"]["extensions"] = nil
    #end

    # check environment variables setted by XCode (when we executed from XCode)
    #xcode_sdk_name = ENV['SDK_NAME']
    #$sdk = xcode_sdk_name if not xcode_sdk_name.nil?
    #$xcode_sdk_dir = ENV['SDK_DIR']
    #xcode_configuration = ENV['CONFIGURATION']
    #$configuration = xcode_configuration if not xcode_configuration.nil?

    make_project_bakup
  end
end

namespace "build" do
  namespace "iphone" do
#    desc "Build iphone rhobundle"
    task :rhobundle => ["config:iphone"] do
      chdir 'platform/iphone'
      rm_rf 'bin'
      rm_rf 'build/Debug-*'
      rm_rf 'build/Release-*'
      
      chdir $startdir
      


      Rake::Task["build:bundle:noxruby"].execute

      Rake::Task["build:iphone:extensions"].execute

      # Store hash
      File.open(File.join($srcdir, "hash"), "w") { |f| f.write(get_dir_hash($srcdir).hexdigest) }
      # Store app name
      File.open(File.join($srcdir, "name"), "w") { |f| f.write($app_config["name"]) }

      Jake.build_file_map( File.join($srcdir, "apps"), "rhofilelist.txt" )  

    end
    
    task :upgrade_package => ["build:iphone:rhobundle"] do
        #puts '$$$$$$$$$$$$$$$$$$'
        #puts 'targetdir = '+$targetdir.to_s
        #puts 'bindir = '+$bindir.to_s
        mkdir_p $targetdir if not File.exists? $targetdir
        zip_file_path = File.join($targetdir, "upgrade_bundle.zip")
        Jake.zip_upgrade_bundle( $bindir, zip_file_path)
    end    

    task :extensions => "config:iphone" do
      ENV['RHO_PLATFORM'] = 'iphone'
      simulator = $sdk =~ /iphonesimulator/
      ENV["PLATFORM_DEVELOPER_BIN_DIR"] ||= $devroot + "/Platforms/" + ( simulator ? "iPhoneSimulator" : "iPhoneOS" ) +
        ".platform/Developer/usr/bin"
      ENV["SDKROOT"] = $sdkroot

      #ENV["SDKROOT"] = $xcode_sdk_dir if not $xcode_sdk_dir.nil?


      ENV["BUILD_DIR"] ||= $startdir + "/platform/iphone/build"
      ENV["TARGET_TEMP_DIR"] ||= $startdir + "/platform/iphone/build/rhorunner.build/#{$configuration}-" +
        ( simulator ? "iphonesimulator" : "iphoneos") + "/rhorunner.build"
      ENV["TEMP_FILES_DIR"] ||= ENV["TARGET_TEMP_DIR"]

      ENV["ARCHS"] ||= simulator ? "i386" : "armv6"
      ENV["RHO_ROOT"] = $startdir

      # added by dmitrys
      ENV["XCODEBUILD"] = $xcodebuild
      ENV["CONFIGURATION"] ||= $configuration
      ENV["SDK_NAME"] ||= $sdk
 
      puts "extpaths: #{$app_config["extpaths"].inspect.to_s}"
      $stdout.flush
      $app_config["extensions"].each do |ext|
        $app_config["extpaths"].each do |p|
	  if p
          extpath = File.join(p, ext, 'ext')
          next unless File.executable? File.join(extpath, 'build')

          puts Jake.run('./build', [], extpath)
          exit 1 unless $? == 0
	 end
        end
      end
    end

    task :restore_xcode_project => ["config:iphone"] do
       restore_project_from_bak 
    end    

    task :setup_xcode_project => ["config:iphone"] do
      restore_project_from_bak

      appname = $app_config["name"] ? $app_config["name"] : "rhorunner"
      vendor = $app_config['vendor'] ? $app_config['vendor'] : "rhomobile"
      bundle_identifier = "com.#{vendor}.#{appname}"
      bundle_identifier = $app_config["iphone"]["BundleIdentifier"] unless $app_config["iphone"]["BundleIdentifier"].nil?
      set_app_bundle_identifier(bundle_identifier)

      set_app_name(appname)

      set_app_version($app_config["version"]) unless $app_config["version"].nil?

      set_app_url_scheme($app_config["iphone"]["BundleURLScheme"]) unless $app_config["iphone"]["BundleURLScheme"].nil?
      set_app_url_name(bundle_identifier)

      set_app_icon(false)
      set_default_images(false)

      if $entitlements == ""
          if $configuration == "Distribution"
              $entitlements = "Entitlements.plist"
          end
      end

      set_signing_identity($signidentity,$provisionprofile,$entitlements.to_s) if $signidentity.to_s != ""
      copy_entitlements_file_from_app
      
    end

#    desc "Build rhodes"
    task :rhodes => ["config:iphone", "build:iphone:rhobundle"] do
      
      saved_name = ''
      saved_version = ''
      saved_identifier = ''
      saved_url_scheme = ''
      saved_url_name = ''

      saved_name = set_app_name($app_config["name"]) unless $app_config["name"].nil?
      saved_version = set_app_version($app_config["version"]) unless $app_config["version"].nil?


      appname = $app_config["name"] ? $app_config["name"] : "rhorunner"
      vendor = $app_config['vendor'] ? $app_config['vendor'] : "rhomobile"
      bundle_identifier = "com.#{vendor}.#{appname}"
      bundle_identifier = $app_config["iphone"]["BundleIdentifier"] unless $app_config["iphone"]["BundleIdentifier"].nil?
      saved_identifier = set_app_bundle_identifier(bundle_identifier)
      
      saved_url_scheme = set_app_url_scheme($app_config["iphone"]["BundleURLScheme"]) unless $app_config["iphone"]["BundleURLScheme"].nil?
      saved_url_name = set_app_url_name(bundle_identifier)

      set_app_icon(true)
      set_default_images(true)

      if $entitlements == ""
          if $configuration == "Distribution"
              $entitlements = "Entitlements.plist"
          end
      end

      set_signing_identity($signidentity,$provisionprofile,$entitlements.to_s) if $signidentity.to_s != ""
      copy_entitlements_file_from_app

      chdir $config["build"]["iphonepath"]

      args = ['build', '-target', 'rhorunner', '-configuration', $configuration, '-sdk', $sdk]

      if $sdk =~ /iphonesimulator/
         args << '-arch'
         args << 'i386'
      end
      
      puts Jake.run($xcodebuild,args)
      ret = $?

      chdir $startdir
      
      set_app_name(saved_name) unless $app_config["name"].nil?
      set_app_version(saved_version) unless $app_config["version"].nil?
      set_app_bundle_identifier(saved_identifier) unless $app_config["iphone"]["BundleIdentifier"].nil?
      set_app_url_scheme(saved_url_scheme) unless $app_config["iphone"]["BundleURLScheme"].nil?
      set_app_url_name(saved_url_name) unless $app_config["iphone"]["BundleIdentifier"].nil?
      
      restore_entitlements_file
      restore_default_images
      restore_app_icon

      unless ret == 0
        puts '************************************'
        puts "ERROR during building by XCode !"
        puts 'XCode return next error code = '+ret.to_s
        exit 1
      end

    end
    
  end
end

namespace "run" do
  namespace "iphone" do

    task :spec => ["clean:iphone",:buildsim] do
 	
      # Run local http server
      $iphonespec = true
      #httpserver = false
      #httpserver = true if File.exist? "#{$app_path}/app/spec/library/net/http/http/fixtures/http_server.rb"

      #if httpserver
      #  require "#{$app_path}/app/spec/library/net/http/http/fixtures/http_server"
      #  NetHTTPSpecs.start_server
      #end

      Jake.before_run_spec

      puts 'kill iPhone Simulator'
      `killall -9  "iPhone Simulator"`
      `killall -9 iphonesim`

      mkdir_p $tmpdir
      log_name  =   File.join($tmpdir, 'logout')
      File.delete(log_name) if File.exist?(log_name)

      $iphone_end_spec = false

      Thread.new {
            # run spec
            rhorunner = File.join($startdir, $config["build"]["iphonepath"],"build/#{$configuration}-iphonesimulator/rhorunner.app")
            #iphonesim = File.join($startdir, 'res/build-tools/iphonesim/build/Release/iphonesim')
            commandis = $iphonesim + ' launch "' + rhorunner + '" ' + $sdkver.gsub(/([0-9]\.[0-9]).*/,'\1') + ' ' + $emulatortarget + ' "' +log_name+'"'
            puts 'use iphonesim tool - open iPhone Simulator and execute our application, also support device family (iphone/ipad)'
            puts 'execute command : ' + commandis
            system(commandis)
            $iphone_end_spec = true
      }

      start = Time.now        

      puts "waiting for log"
      
        while (!File.exist?(log_name)) && (!$iphone_end_spec)
            sleep(1)
        end

        puts "start read log"
        
        #$iphone_end_spec = false

        while !$iphone_end_spec do
            io = File.new(log_name, "r")
        
            io.each do |line|
                puts line
                
                if line.class.method_defined? "valid_encoding?" 
                    $iphone_end_spec = !Jake.process_spec_output(line) if line.valid_encoding?
                else 
                    $iphone_end_spec = !Jake.process_spec_output(line)
                end                    
               
                break if $iphone_end_spec
            end
            io.close
            
            sleep(5) unless $iphone_end_spec
        end
      puts 'spec logging is finished'

      Jake.process_spec_results(start)

      File.delete(log_name) if File.exist?(log_name)

      $stdout.flush

      puts 'kill iPhone Simulator'
      `killall -9  "iPhone Simulator"`
      `killall -9 iphonesim`

      $stdout.flush

      #NetHTTPSpecs.stop_server if httpserver

      unless $dont_exit_on_failure
        exit 1 if $total.to_i==0
        exit $failed.to_i
      end
    end

    task :spec_old => ["clean:iphone",:buildsim] do

      sdkroot = $devroot + "/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator" +
                $sdk.gsub(/iphonesimulator/,"") + ".sdk"

      old_user_home = ENV["CFFIXED_USER_HOME"]
      old_dyld_root = ENV["DYLD_ROOT_PATH"]
      old_dyld_framework = ENV["DYLD_FRAMEWORK_PATH"]
      old_iphone_simulator = ENV["IPHONE_SIMULATOR_ROOT"]

      ENV["CFFIXED_USER_HOME"] = $simrhodes
      ENV["DYLD_ROOT_PATH"] = sdkroot
      ENV["DYLD_FRAMEWORK_PATH"] = sdkroot + "/System/Library/Frameworks"
      ENV["IPHONE_SIMULATOR_ROOT"] = sdkroot

      command = '"' + $simrhodes + '/rhorunner.app/rhorunner"' + " -RegisterForSystemEvents"

      #if someone runs against the wrong app, kill after 120 seconds
      Thread.new {
        sleep 300
        `killall -9 rhorunner`
      }

      `killall -9 rhorunner`

      # Run local http server
      $iphonespec = true
      httpserver = false
      httpserver = true if File.exist? "#{$app_path}/app/spec/library/net/http/http/fixtures/http_server.rb"

      if httpserver
        require "#{$app_path}/app/spec/library/net/http/http/fixtures/http_server"
        NetHTTPSpecs.start_server
      end

      Jake.before_run_spec

      start = Time.now
      io = IO.popen(command)
      io.each do |line|
        Jake.process_spec_output(line)
      end

      Jake.process_spec_results(start)

      $stdout.flush

      NetHTTPSpecs.stop_server if httpserver

      ENV["CFFIXED_USER_HOME"] = old_user_home
      ENV["DYLD_ROOT_PATH"] = old_dyld_root
      ENV["DYLD_FRAMEWORK_PATH"] = old_dyld_framework
      ENV["IPHONE_SIMULATOR_ROOT"] = old_iphone_simulator
      unless $dont_exit_on_failure
        exit 1 if $total.to_i==0
        exit $failed.to_i
      end
    end



    task :phone_spec do
      Jake.run_spec_app('iphone','phone_spec')
    end

    task :framework_spec do
      Jake.run_spec_app('iphone','framework_spec')
    end

    task :allspecs do
      rm_rf basedir + "/faillog.txt"
      $dont_exit_on_failure = true
      Rake::Task['run:iphone:phone_spec'].invoke
      Rake::Task['run:iphone:framework_spec'].invoke
      failure_output = ""
      if $failed.to_i > 0
        failure_output = ""
        failure_output += "phone_spec failures:\n\n" + File.open(app_expanded_path('phone_spec') + "/faillog.txt").read if
          File.exist?(app_expanded_path('phone_spec') + "/faillog.txt")
        failure_output += "framework_spec failures:\n\n" + File.open(app_expanded_path('framework_spec') + "/faillog.txt").read if
          File.exist?(app_expanded_path('framework_spec') + "/faillog.txt")
        chdir basedir
        File.open("faillog.txt", "w") { |io| failure_output.each {|x| io << x }  }
      end
      puts "Agg Total: #{$total}"
      puts "Agg Passed: #{$passed}"
      puts "Agg Failed: #{$failed}"
      exit 1 if $total.to_i==0
      exit $failed.to_i
    end

    desc "Run application on RhoSimulator"    
    task :rhosimulator => ["config:set_iphone_platform","config:common"] do        
        $rhosim_config = "platform='iphone'\r\n"
        
        Rake::Task["run:rhosimulator"].invoke
    end

    task :rhosimulator_debug => ["config:set_iphone_platform","config:common"] do        
        $rhosim_config = "platform='iphone'\r\n"
        
        Rake::Task["run:rhosimulator_debug"].invoke
    end

    #<<<<<<<<<
    task :get_log => ["config:iphone"] do
      puts $simapppath
       $sdkver = $emulator_version.to_s unless $emulator_version.nil?
      
       simapp = File.join($simdir, $sdkver, "Applications")
      
       Dir.glob(File.join($simdir, $sdkver, "Applications", "*")).each do |simapppath|
           need_rm = true if File.directory? simapppath
           if File.exists?(File.join(simapppath, 'rhorunner.app', 'name'))
             name = File.read(File.join(simapppath, 'rhorunner.app', 'name'))
             puts "found app name: #{name}"
             guid = File.basename(simapppath)
             puts "found guid: #{guid}"
             if name == $app_config['name']
                 $guid = guid
                 need_rm = false
             end
         end
         rm_rf simapppath if need_rm
         rm_rf simapppath + ".sb" if need_rm
       end
      
       simapp = File.join($simdir, $emulator_version, "Applications")
       
       
       rholog = simapp + "/" + $guid + "/Library/Caches/Private Documents/rholog.txt"
       puts "log_file=" + rholog
    end 

  end
  
  task :buildsim => ["config:iphone", "build:iphone:rhodes"] do
    
     unless $sdk =~ /^iphonesimulator/
       puts "SDK must be one of the iphonesimulator sdks to run in the iphone simulator"
       exit 1       
     end
     `killall "iPhone Simulator"`


      puts 'kill iPhone Simulator'
      `killall -9  "iPhone Simulator"`
      `killall -9 iphonesim`

     use_old_scheme = ($emulatortarget != 'iphone') && ($emulatortarget != 'ipad')


     $sdkver = $sdk.gsub(/^iphonesimulator/, '')
     # Workaround: sometimes sdkver could differ from emulator version.
     # Example: iPhone SDK 4.0.1. In this case sdk is still iphonesimulator4.0 but version of simulator is 4.0.1
     $sdkver = $emulator_version.to_s unless $emulator_version.nil?



     #if use_old_scheme

          elements = []
          binplist = File.join(ENV['HOME'], 'Library', 'Preferences', 'com.apple.iphonesimulator.plist')
          xmlplist = '/tmp/iphone.plist'
          if File.exists? binplist
              `plutil -convert xml1 -o #{xmlplist} #{binplist}`

              elements = []
              doc = REXML::Document.new(File.new(xmlplist))
              nextignore = false
              doc.elements.each('plist/dict/*') do |element|
                  if nextignore
                      nextignore = false
                      next
                  end
                  if element.name == 'key'
                      if element.text == 'currentSDKRoot' or element.text == 'SimulateDevice'
                          nextignore = true
                          next
                      end
                  end
         
                  elements << element
              end
         end

         e = REXML::Element.new 'key'
         e.text = 'SimulateDevice'
         elements << e
         e = REXML::Element.new 'string'
         e.text = $sdkver == '3.2' ? 'iPad' : 'iPhone'
         elements << e
         e = REXML::Element.new 'key'
         e.text = 'currentSDKRoot'
         elements << e
         e = REXML::Element.new 'string'
         e.text = $sdkroot
         elements << e

         File.open(xmlplist, 'w') do |f|
             f.puts "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
             f.puts "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"
             f.puts "<plist version=\"1.0\">"
             f.puts "<dict>"
             elements.each do |e|
                 f.puts "\t#{e.to_s}"
             end
             f.puts "</dict>"
             f.puts "</plist>"
         end

         `plutil -convert binary1 -o #{binplist} #{xmlplist}`

         rhorunner = $config["build"]["iphonepath"] + "/build/#{$configuration}-iphonesimulator/rhorunner.app"
         puts "rhorunner: #{rhorunner}"

         puts "our app name: #{$app_config['name']}"
         puts "simdir: #{$simdir}"

         Dir.glob(File.join($simdir, $sdkver, "Applications", "*")).each do |simapppath|
             need_rm = true if File.directory? simapppath
             if File.exists?(File.join(simapppath, 'rhorunner.app', 'name'))
                 name = File.read(File.join(simapppath, 'rhorunner.app', 'name'))
                 puts "found app name: #{name}"
                 guid = File.basename(simapppath)
                 puts "found guid: #{guid}"
                 if name == $app_config['name']
                     $guid = guid
                     need_rm = false
                 end
             end
             rm_rf simapppath if need_rm
             rm_rf simapppath + ".sb" if need_rm
         end

         puts "app guid: #{$guid}"

         mkdir_p File.join($simdir, $sdkver)

         simapp = File.join($simdir, $sdkver, "Applications")
         simlink = File.join($simdir, $sdkver, "Library", "Preferences")

         $simrhodes = File.join(simapp, $guid)
         chmod 0744, File.join($simrhodes, "rhorunner.app", "rhorunner") unless !File.exists?(File.join($simrhodes, "rhorunner.app", "rhorunner"))

         mkdir_p File.join($simrhodes, "Documents")
         mkdir_p File.join($simrhodes, "Library", "Preferences")
         mkdir_p File.join($simrhodes, "Library", "Caches", "Private Documents")

         rm_rf File.join($simrhodes, 'rhorunner.app')
         cp_r rhorunner, $simrhodes
         ['com.apple.PeoplePicker.plist', '.GlobalPreferences.plist'].each do |f|
             `ln -f -s "#{simlink}/#{f}" "#{$simrhodes}/Library/Preferences/#{f}"`
         end

#>>>>>>>>>>
        #`echo "#{$applog}" > "#{$simrhodes}/Documents/rhologpath.txt"`
        rholog = simapp + "/" + $guid + "/Library/Caches/Private Documents/rholog.txt"


        simpublic = simapp + "/" + $guid + "/Documents/apps/public"
        apppublic = $app_path + "/sim-public-#{$sdkver}"


        apprholog = $app_path + "/rholog-#{$sdkver}.txt"
        apprholog = $app_path + "/" + $app_config["applog"] if $app_config["applog"]

        rm_f apprholog
        rm_f apppublic
        `ln -f -s "#{simpublic}" "#{apppublic}"`
        `ln -f -s "#{rholog}" "#{apprholog}"`
        `echo > "#{rholog}"`
        f = File.new("#{simapp}/#{$guid}.sb","w")
        f << "(version 1)\n(debug deny)\n(allow default)\n"
        f.close
     #end    
  end

  # split this off separate so running it normally is run:iphone
  # testing we will not launch emulator directly
  desc "Builds everything, launches iphone simulator"
  task :iphone => :buildsim do
    
    rhorunner = File.join($startdir, $config["build"]["iphonepath"],"build/#{$configuration}-iphonesimulator/rhorunner.app")
    commandis = $iphonesim + ' launch "' + rhorunner + '" ' + $sdkver.gsub(/([0-9]\.[0-9]).*/,'\1') + ' ' + $emulatortarget

    thr = Thread.new do
       puts 'start thread with execution of application' 
       if ($emulatortarget != 'iphone') && ($emulatortarget != 'ipad')
           puts  'use old execution way - just open iPhone Simulator'
           system("open \"#{$sim}/iPhone Simulator.app\"")
       else
           puts 'use iphonesim tool - open iPhone Simulator and execute our application, also support device family (iphone/ipad)'
           system(commandis)
       end
    end
    
    thr.join
  
    puts "end build iphone app"  
    exit
  end
  
  task :allspecs do
    $dont_exit_on_failure = true
    Rake::Task['run:iphone:phone_spec'].invoke
    Rake::Task['run:iphone:framework_spec'].invoke
    failure_output = ""
    if $failed.to_i > 0
      failure_output = ""
      failure_output += "phone_spec failures:\n\n" + File.open(app_expanded_path('phone_spec') + "/faillog.txt").read if
        File.exist?(app_expanded_path('phone_spec') + "/faillog.txt")
      failure_output += "framework_spec failures:\n\n" + File.open(app_expanded_path('framework_spec') + "/faillog.txt").read if
        File.exist?(app_expanded_path('framework_spec') + "/faillog.txt")
      chdir basedir
      File.open("faillog.txt", "w") { |io| failure_output.each {|x| io << x }  }
    end
    puts "Agg Total: #{$total}"
    puts "Agg Passed: #{$passed}"
    puts "Agg Failed: #{$failed}" 
    exit 1 if $total.to_i==0
    exit $failed.to_i
  end
  
end

namespace "clean" do
  desc "Clean iphone"
  task :iphone => ["clean:iphone:all"]
  namespace "iphone" do
#    desc "Clean rhodes binaries"
    task :rhodes => ["config:iphone"] do 

      app_path = File.join($app_path, 'bin', 'target', 'iOS')
      rm_rf app_path

      chdir $config["build"]["iphonepath"]
    
      args = ['clean', '-target', 'rhorunner', '-configuration', $configuration, '-sdk', $sdk]
      puts Jake.run($xcodebuild,args)
      unless $? == 0
        puts "Error cleaning"
        exit 1
      end
      chdir $startdir
      
      chdir 'platform/iphone'
       rm_rf 'build/Debug-*'
       rm_rf 'build/Release-*'
      chdir $startdir

      found = true

      while found do
        found = false
        Find.find($simdir) do |path|
          if File.basename(path) == "rhorunner.app"
            $guid = File.basename(File.dirname(path))
            found = true
          end
        end

        if found
         Dir.glob($simdir + '*').each do |sdk|
          simapp = sdk + "/Applications"
          simrhodes = File.join(simapp,$guid)
          rm_rf simrhodes
          rm_rf simrhodes + ".sb"
         end
        end
      end
    end

#    desc "Clean rhobundle"
    task :rhobundle => ["config:iphone"] do
      rm_rf $bindir
    end

    task :all => ["clean:iphone:rhodes", "clean:iphone:rhobundle"]
  end
end




namespace "device" do
  namespace "iphone" do
    desc "Builds and signs iphone for production"
    task :production => ["config:iphone", "build:iphone:rhodes"] do
    
    #copy build results to app folder
    
    app_path = File.join($app_path, 'bin', 'target', 'iOS', $sdk, $configuration)
    
    iphone_path = $config["build"]["iphonepath"]    
    if $sdk =~ /iphonesimulator/
       iphone_path = File.join(iphone_path, 'build', $configuration+'-iphonesimulator')
    else 
       iphone_path = File.join(iphone_path, 'build', $configuration+'-iphoneos')
    end
    appname = $app_config["name"]
    if appname == nil
       appname = 'rhorunner'
    end
    src_file = File.join(iphone_path, 'rhorunner.app')
    dst_file = File.join(app_path, appname+'.app')
    
    rm_rf dst_file
    rm_rf app_path

    mkdir_p app_path

    puts 'copy result build package to application target folder ...'    
    cp_r src_file, dst_file
    make_app_info 
    prepare_production_ipa(app_path, appname)
    prepare_production_plist(app_path, appname)
    copy_all_png_from_icon_folder_to_product(app_path)
    
    end
  end

end
