# -*- encoding: utf-8 -*-
#require "lib/rhodes.rb"

Gem::Specification.new do |s|
  s.name = %q{rhodes}
  s.version = File.read('version').chomp

  s.required_rubygems_version = Gem::Requirement.new(">= 0") if s.respond_to? :required_rubygems_version=
  s.authors = ["Rhomobile"]
  s.date = Time.now
  s.description = %q{Rhodes mobile framework}
  s.email = %q{dev@rhomobile.com}
  s.extra_rdoc_files = ["README.md", "LICENSE"]
  files = Array.new
  IO.read("Manifest.txt").each_line {|x| files << x.chomp}
  s.files =  files
  s.homepage = %q{http://www.rhomobile.com}
  s.rdoc_options = ["--inline-source", "--charset=UTF-8"]
  s.require_paths = ["lib"]
  s.rubyforge_project = %q{rhodes}
  s.rubygems_version = %q{1.3.5}
  s.add_dependency('templater', '>= 0.5.0')
  s.add_dependency('rake', '>= 0.9.2')
  s.add_dependency('activesupport', '~> 2.3.5')
  s.add_dependency('uuid', '>= 2.3.2')
  #s.add_dependency('rdoc', '~> 3.6.1')
  s.executables << 'rhogen'
  s.executables << 'rhodes'
  s.executables << 'upgrade-rhodes-app'
  s.executables << 'rhodes-setup'
  s.executables << 'set-rhodes-sdk'
  s.executables << 'migrate-rhodes-app'
  s.executables << 'get-rhodes-info'
  #this is a hack, we dont have an extension, but we want to run postinstall code. It's done via the 'building' of an extension
  s.extensions << 'ext/extconf.rb'
  s.summary = %q{The Rhodes framework is the easiest way to develop NATIVE apps with full device capabilities (GPS, PIM, camera, etc.) for any smartphone.}

  if s.respond_to? :specification_version then
    current_version = Gem::Specification::CURRENT_SPECIFICATION_VERSION
    s.specification_version = 2

    if Gem::Version.new(Gem::RubyGemsVersion) >= Gem::Version.new('1.2.0') then
      s.add_runtime_dependency(%q<diff-lcs>, [">= 1.1.2"])
    else
      s.add_dependency(%q<diff-lcs>, [">= 1.1.2"])
    end
  else
    s.add_dependency(%q<diff-lcs>, [">= 1.1.2"])
  end
end

