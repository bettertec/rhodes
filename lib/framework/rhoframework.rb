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

if defined?( RHO_WP7 )
    RHO_RB_EXT = '.rb'
    RHO_ERB_EXT = '_erb' + RHO_RB_EXT
    RHO_APPS_DIR = 'apps/'
elsif defined?( RHODES_EMULATOR )
    RHO_RB_EXT = '.rb'
    RHO_ERB_EXT = '.erb'
    RHO_APPS_DIR = ''
    RHO_EMULATOR_DIR = 'rhosimulator'    
    module Kernel   
	    def eval_compiled_file(fname, bind)
	        require 'erb'
	        
	        rhosim_platform = Rho::RhoConfig.rhosim_platform
	        if rhosim_platform && rhosim_platform.length() > 0 
	            dot = fname.rindex('.')
	            
	            if dot
	                fname1 = fname.dup
                    fname1.insert(dot, "." + rhosim_platform )
	            else
	                fname1 = fname + "."  + rhosim_platform
	            end
	            
                fname = fname1 if File.exist?(fname1)	            
	        end

	        puts "eval_compiled_file : #{fname}"
	        
	        strFile = IO.read(fname)
	        code = ERB.new(strFile).src
	        
		    eval(code, bind)
	    end
    end
else
    RHO_RB_EXT = '.iseq'
    RHO_ERB_EXT = '_erb' + RHO_RB_EXT
    RHO_APPS_DIR = 'apps/'
end

module Rho
  def self.file_exist?(fname)
      return File.exist_injar?(fname) if defined? RHO_ME

	  return __rho_exist_in_resources(fname) if defined? RHO_WP7
	              
      File.exist?(fname)
  end
end

if defined? RHO_WP7

module Kernel   
	def __rhoGetCurrentDir
		""
	end

	def __rhoGetRuntimeDir
		""
	end

	alias :orig_require :require

	def require(file_name)
		 file_name += '.rb' unless file_name.end_with?('.rb')
		 orig_require( file_name )
	end

	alias :orig_load :load

	def load(file_name)
		 file_name += '.rb' unless file_name.end_with?('.rb')
		 orig_load( file_name )
	end

	def eval_compiled_file(fname, bind)
	    code = IO.read(fname)
	    code.force_encoding('utf-8')
		eval(code, bind)
	end
end

end

# Mutex
class Mutex
  def synchronize
    self.lock
    begin
      yield
    ensure
      self.unlock rescue nil
    end
  end
end

# Thread
class Thread
  MUTEX_FOR_THREAD_EXCLUSIVE = Mutex.new
  def self.exclusive
    MUTEX_FOR_THREAD_EXCLUSIVE.synchronize{
      yield
    }
  end
end

module SignatureTool
    def SignatureTool.take_signature( callback, image_fmt)
        
        Rho::SignatureCapture.take(callback, :imageFormat => image_fmt)
    end
end

begin
	if defined? RHO_WP7
		require 'rhosystem'
	end

    if defined? RHO_ME
	    require 'rationalME'
    else	
        require 'rational'
    end
    
    require 'rhom/rhom_db_adapter'
	
    require 'rhom/rhom_object'
    require 'rhofsconnector'
	
    require 'rhom/rhom_object_factory'
    
    require 'rhom/rhom'
    require 'rhom'

    require 'rho/rhoapplication'

    require 'indifferent_access'
    require 'rho/rhosupport'    
    require 'rho/rhocontroller'    

    require 'rho/rho'
    require 'rho/render'
    
    require 'rho'

if defined?( RHODES_EMULATOR )
    require 'debugger'
end
	
    puts 'RHO loaded'
    RHO_FRAMEWORK = Rho::RHO.new
rescue Exception => e
    trace_msg = e.backtrace.join("\n")
    puts 'Create RHO framework failed: ' + e.inspect + ";Trace: #{trace_msg}"
end    

