require 'local_server'

describe "AsyncHttp" do

    after(:all) do
        file_name = File.join(Rho::RhoApplication::get_base_app_path(), 'test.jpg')
        File.delete(file_name) if File.exists?(file_name)

        file_name = File.join(Rho::RhoApplication::get_app_path('DataTemp'), 'test_log.txt')
        File.delete(file_name) if File.exists?(file_name)		
        
    end
    
    it "should http get" do
        res = Rho::AsyncHttp.get(
          :url => 'http://www.apache.org/licenses/LICENSE-2.0' )
        
        #puts "res : #{res}"  
        
        res['status'].should == 'ok'
        res['headers']['content-type'].should ==  'text/plain; charset=utf-8'
        res['body'].should_not be_nil

        # www.apache.org response with gzipped body if client declare 'Accept-Encoging: gzip'
        # (our network implementation for iPhone and Android does).
        # It means that content-length header will contain value less than
        # body length because body we have on application level is already decoded
        # This is why following two lines commented out
        #res['headers']['content-length'].should == 11358
        #res['body'].length.should == res['headers']['content-length'].to_i
        res['body'].length.should == 11358
    end

    it "should http post" do
        
        #TODO: post_test
    end
if !defined?(RHO_WP7)
    it "should http download" do

        file_name = File.join(Rho::RhoApplication::get_base_app_path(), 'test.jpg')
        File.delete(file_name) if File.exists?(file_name)
        File.exists?(file_name).should == false

        res = Rho::AsyncHttp.download_file(
          :url => 'http://www.rhomobile.com/wp-content/themes/rhomobile_theme/images/misc/ruby_app.jpg',
          :filename => file_name )
        puts "res : #{res}"  
        
        res['status'].should == 'ok'
        res['headers']['content-length'].to_i.should ==  14451
        res['headers']['content-type'].should == 'image/jpeg'

        File.exists?(file_name).should == true
        orig_len = File.size(file_name)
        orig_len.should == res['headers']['content-length'].to_i

        #check that in case of one more download, files keeps the same        
        res = Rho::AsyncHttp.download_file(
          :url => 'http://www.rhomobile.com/wp-content/themes/rhomobile_theme/images/misc/ruby_app.jpg',
          :filename => file_name )
        puts "res : #{res}"  
        
        res['status'].should == 'ok'
        res['headers']['content-length'].to_i.should == 12554
        res['http_error'].should == '206'
        #res['headers']['content-type'].should == 'image/jpeg'

        File.exists?(file_name).should == true
        File.size(file_name).should == orig_len

        #check that in case of network error, files keeps the same        
        res = Rho::AsyncHttp.download_file(
          :url => 'http://www.rhomobile.com/wp-content/themes/rhomobile_theme/images/misc/ruby_app_BAD.jpg',
          :filename => file_name )
        puts "res : #{res}"  
        res['status'].should == 'error'
        res['http_error'].should == '404'

        File.exists?(file_name).should == true
        File.size(file_name).should == orig_len
    end
end
    it "should http upload" do
        
        server = 'http://rhologs.heroku.com'
        
        file_name = File.join(Rho::RhoApplication::get_app_path('DataTemp'), 'test_log.txt')
        File.exists?(file_name).should ==  true

        res = Rho::AsyncHttp.upload_file(
          :url => server + "/client_log?client_id=&device_pin=&log_name=uptest",
          :filename => file_name )
        #puts "res : #{res}"  
        
        res['status'].should == 'ok'
        File.exists?(file_name).should ==  true
    end

    it "should http upload" do
        
        server = 'http://rhologs.heroku.com'
		dir_name = Rho::RhoApplication::get_app_path('DataTemp')
		Dir.mkdir(dir_name) unless Dir.exists?(dir_name)
        
        file_name = File.join(dir_name, 'test_log.txt')
        puts " file_name : #{file_name}"
        File.open(file_name, "w"){|f| puts "OK"; f.write("******************THIS IS TEST! REMOVE THIS FILE! *******************")}

        res = Rho::AsyncHttp.upload_file(
          :url => server + "/client_log?client_id=&device_pin=&log_name=uptest",
          :filename => file_name )
          #optional parameters:
          #:filename_base => "phone_spec_file",
          #:name => "phone_spec_name" )
        
        res['status'].should == 'ok'
        File.exists?(file_name).should ==  true
    end

    it "should decode chunked body" do

      host = SPEC_LOCAL_SERVER_HOST
      port = SPEC_LOCAL_SERVER_PORT
      puts "+++++++++++++++++++ chunked test: #{host}:#{port}"
      res = Rho::AsyncHttp.get :url => "http://#{host}:#{port}/chunked"
      res['status'].should == 'ok'
      res['body'].should_not be_nil
      res['body'].should == "1234567890"
    end

    it "should send custom command" do
        
        res = Rho::AsyncHttp.get(
          :url => 'http://www.apache.org/licenses/LICENSE-2.0',
          :http_command => 'PUT' )
        
        #puts "res : #{res}"  
        res['http_error'].should == '405'
        res['body'].index('The requested method PUT is not allowed for the URL').should_not be_nil
        
        res = Rho::AsyncHttp.post(
          :url => 'http://www.apache.org/licenses/LICENSE-2.0',
          :http_command => 'PUT' )
        
        #puts "res : #{res}"  
        res['http_error'].should == '405'
        res['body'].index('The requested method PUT is not allowed for the URL').should_not be_nil
        
    end    

    it "should upload with body" do
        
        server = 'http://rhologs.heroku.com'
        
        file_name = File.join(Rho::RhoApplication::get_app_path('app'), 'Data/test_log.txt')
        File.open(file_name, "w"){|f| f.write("******************THIS IS TEST! REMOVE THIS FILE! *******************")}

        res = Rho::AsyncHttp.upload_file(
          :url => server + "/client_log?client_id=&device_pin=&log_name=uptest",
          :filename => file_name,
          :file_content_type => "application/octet-stream",
          :filename_base => "phone_spec_file",
          :name => "phone_spec_name",
          
          :body => "upload test",
          :headers => {"content-type"=>"plain/text"}
           )
        #puts "res : #{res}"  
        
        res['status'].should == 'ok'
        File.exists?(file_name).should == true
    end

    it "should upload miltiple" do
        
        server = 'http://rhologs.heroku.com'
        
        file_name = File.join(Rho::RhoApplication::get_app_path('app'), 'Data/test_log.txt')
        File.open(file_name, "w"){|f| f.write("******************THIS IS TEST! REMOVE THIS FILE! *******************")}

        res = Rho::AsyncHttp.upload_file(
          :url => server + "/client_log?client_id=&device_pin=&log_name=uptest",
          :multipart => [
              { 
                :filename => file_name,
                :filename_base => "phone_spec_file",
                :name => "phone_spec_name",
                :content_type => "application/octet-stream"
              },
              {
                :body => "upload test",
                :name => "phone_spec_bodyname",
                :content_type => "plain/text"
              }
           ]
        )
        #puts "res : #{res}"  
        
        res['status'].should == 'ok'
        File.exists?(file_name).should == true
    end

    it "should send https request" do
            
        res = Rho::AsyncHttp.get(
          :url => 'https://rhologs.heroku.com' )
        
        puts "res : #{res}"  
        
        res['status'].should == 'ok'
        
        http_error = res['http_error'].to_i if res['http_error']
        if http_error == 301 || http_error == 302 #redirect
            res2 = Rho::AsyncHttp.get( :url => res['headers']['location'] )
            
            res2['status'].should == 'ok'
        end    
        
    end

end    
