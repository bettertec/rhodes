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

require 'rho/render'
require 'rho/rhosupport'
require 'rho/rhoviewhelpers'

module Rho
  class RhoController
  	attr_accessor :menu

    @@before = nil
    def rho_info(str)
      RhoLog.info("RHO " + self.class.to_s, str)
    end

    def rho_error(str)
      RhoLog.error("RHO " + self.class.to_s, str)
    end

    def app_info(str)
      RhoLog.info("APP " + self.class.to_s, str)
    end

    def app_error(str)
      RhoLog.error("APP " + self.class.to_s, str)
    end

    def default_action
      return Hash['GET','show','PUT','update','POST','update',
        'DELETE','delete'][@request['request-method'].upcase] unless @request['id'].nil?
      return Hash['GET','index','POST','create'][@request['request-method'].upcase]
    end

    def self.process_rho_object(params)
      if params['rho_callback'] && params['__rho_object']
        hashObjs = params['__rho_object']
        
        hashObjs.each do |name,index|
            if name == '__rho_inline'
                params.merge!( __rhoGetCallbackObject(index.to_i()) )
                
                barcodeModule = Object.const_get('Barcode') if Object.const_defined?('Barcode')
                if barcodeModule && barcodeModule.respond_to?( :rho_process_moto_callback )
                    barcodeModule.rho_process_moto_callback(params)
                end
                cameraModule = Object.const_get('Camera') if Object.const_defined?('Camera')
                if cameraModule && cameraModule.respond_to?( :rho_process_moto_callback )
                    cameraModule.rho_process_moto_callback(params)
                end
                
            else
                params[name] = __rhoGetCallbackObject(index.to_i())
            end    
        end
        
        params.delete('__rho_object')
      end
    end
    
    def self.before(&block)
      @@before = {} unless @@before
      @@before[self.to_s] = block if block_given?
    end
    
    def serve(application,object_mapping,req,res)
      @request, @response = req, res
      @object_mapping = object_mapping
      @params = RhoSupport::query_params req
      @rendered = false
      @redirected = false

      RhoController.process_rho_object(@params)
      
      if @@before and @@before[self.class.to_s] and not @params['rho_callback']
        @@before[self.class.to_s].call(@params,@request) 
      end
      
      act = req['action'].nil? ? default_action : req['action']
      if self.respond_to?(act)
        res = send req['action'].nil? ? default_action : req['action']
      else
        called_action = @request['action'].nil? ? default_action : @request['action']
        unless Rho::file_exist?(@request[:modelpath]+called_action.to_s+RHO_ERB_EXT)
          rho_error( "Action '#{act}' does not exist in controller or has private access."  )
          res = render :string => "<font size=\"+4\"><h2>404 Not Found.</h2>The action <i>#{called_action}</i> does not have a view or a controller</font>"
        end
      end
      
      if @params['rho_callback']
        res = "" unless res.is_a?(String)
        return res
      end
        
      res = render unless @rendered or @redirected
        
      application.set_menu(@menu, @back_action) if @back_action
      #@menu = nil
      #@back_action = nil;
  	  res
    end

    # Returns true if the request's header contains "XMLHttpRequest".
    def xml_http_request?
      return false if !@request || !@request['headers'] || !@request['headers']['X-Requested-With']
      test2 = /XMLHttpRequest/i.match(@request['headers']['X-Requested-With'])
      not test2.nil?
    end
    alias xhr? :xml_http_request?

    def redirect(url_params = {},options = {})
      if @params['rho_callback']
        rho_error( "redirect call in callback. Call WebView.navigate instead" ) 
        return ""
      end  
    
      @redirected = true
      @response['status'] = options['status'] || 302 
      @response['headers']['Location'] = url_for(url_params)
      @response['message'] = options['message'] || 'Moved temporarily'
      return ''
    end
    
    def strip_braces(str=nil)
      str ? str.gsub(/\{/,"").gsub(/\}/,"") : nil
    end

  end # RhoController
end # Rho