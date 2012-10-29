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

require 'bsearch'

module Rho
	class RhoContact

            AUTHORIZATION_STATUS_NOT_DETERMINED = 'Not Determined'            
            AUTHORIZATION_STATUS_RESTRICTED = 'Restricted'            
            AUTHORIZATION_STATUS_DENIED = 'Denied'            
            AUTHORIZATION_STATUS_AUTHORIZED = 'Authorized'            

            def self.get_authorization_status
                    result = AUTHORIZATION_STATUS_AUTHORIZED
                    if System::get_property('platform') == "ANDROID" or System::get_property('platform') == "APPLE"
                        result = Phonebook::get_authorization_status()
                    end
                    result
            end
		class << self
            def open_phonebook()
                return Phonebook::openPhonebook
            end
            def close_phonebook(phonebook)
                Phonebook::closePhonebook(phonebook)
            end  
            def find(*args)
                result = nil
                phonebook = args[1][:phonebook] if args[1]
                pb = Phonebook::openPhonebook if phonebook.nil?

                if pb.nil?
                    puts "Can't open phonebook"
                elsif args.first.nil? or args.length == 0
                    puts "There are no arguments to find contacts"
                elsif args.first == :all or args.first == :first
                    if System::get_property('platform') == "ANDROID" or System::get_property('platform') == "APPLE"
                        result = Phonebook::getRecords(pb, args[1])
                    else
                        result = Phonebook::allRecords(pb)
                    end
		        elsif args.first == :count
                    if System::get_property('platform') == "ANDROID" or System::get_property('platform') == "APPLE"
                        result = Phonebook::countRecords(pb, args[1])
                    else
                        result = Phonebook::countRecords(pb);
                    end
                else
                    result = Phonebook::getRecord(pb, args.first)
                end

                Phonebook::closePhonebook(pb) if phonebook.nil?
                result
			end

			def create!(properties, phonebook = nil)
				pb = phonebook
				if phonebook == nil
					pb = Phonebook::openPhonebook
				end 
				unless pb.nil?
					record = Phonebook::createRecord(pb)
					if record.nil?
						puts "Can't find record " + properties['id']
					else
						properties.each do |key,value|
							Phonebook::setRecordValue(record,key,value)
						end
						Phonebook::addRecord(pb,record)
					end
					if phonebook == nil
						Phonebook::closePhonebook(pb)
					end
				end
			end

			def destroy(recordId, phonebook = nil)
                                pb = phonebook
                if phonebook == nil
                    pb = Phonebook::openPhonebook
                end
				unless pb.nil?
					record = Phonebook::openRecord(pb,recordId)
					if record.nil?
						puts "Can't find record " + recordId
					else
						Phonebook::deleteRecord(pb,record)
					end
					if phonebook == nil
						Phonebook::closePhonebook(pb)
					end
				end
				return record
			end

			def update_attributes(properties, phonebook = nil)
                                pb = phonebook
                                if phonebook == nil
					pb = Phonebook::openPhonebook
                                end 
				unless pb.nil?
					record = Phonebook::openRecord(pb, properties['id'])
					if record.nil?
						puts "Can't find record " + properties['id']
					else
						properties.each do |key,value|
							Phonebook::setRecordValue(record,key,value)
						end
						Phonebook::saveRecord(pb,record)
					end
					if phonebook == nil
						Phonebook::closePhonebook(pb)
					end
				end
			end

			# Examples of how to use select method:
			#
    		# selected = Rho::RhoContact.select('first_name' => 'David') { |x| x[1]['last_name']=='Taylor' }
			# ==> returns record(s) of the David Taylor
			#
    		# selected = Rho::RhoContact.select('first_name' => 'Kate')
			# ==> Returns all records of Kate
			#
    		# selected = Rho::RhoContact.select('last_name' => 'User') do |x|
    		# 	x[1]['first_name']=='Test' and x[1]['company_name']=="rhomobile"
    		# end
			# ==> returns all records of the Test User from the company rhomobile
			#
			def select(index, &block)
				key, value = index.keys[0], index.values[0]
				if @contacts.nil? or @key != key
					@key, @contacts = key, find(:all).to_a.sort! {|x,y| x[1][key] <=> y[1][key] }
				end
				found = @contacts[@contacts.bsearch_range {|x| x[1][key] <=> value}]
				unless found.nil? or block.nil?
					return found.select(&block)
				end
				return found
			end

			def select_by_name(first_last_name, &block)
				if @contacts.nil?
					@contacts = find(:all).to_a.sort! do |x,y|
						xname = (x[1]['first_name'] or "") + " " + (x[1]['last_name'] or "")
						yname = (y[1]['first_name'] or "") + " " + (y[1]['last_name'] or "")
						xname <=> yname
					end
				end

				range = @contacts.bsearch_range do |x|
					(x[1]['first_name'] or "") + " " + (x[1]['last_name'] or "") <=> first_last_name
				end
				found = @contacts[range]
				unless found.nil? or block.nil?
					return found.select(&block)
				end
				return found
			end

		end #<< self
	end # class RhoContact
end # module Rho
