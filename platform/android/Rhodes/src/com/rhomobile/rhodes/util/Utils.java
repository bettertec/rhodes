/*------------------------------------------------------------------------
* (The MIT License)
* 
* Copyright (c) 2008-2011 Rhomobile, Inc.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
* 
* http://rhomobile.com
*------------------------------------------------------------------------*/

package com.rhomobile.rhodes.util;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.content.res.AssetManager;

public class Utils {
	
	public static class FileSource {
		
		String[] list(String dir) throws IOException {
			return new File(dir).list();
		}
		
		InputStream open(String file) throws FileNotFoundException, IOException {
			return new FileInputStream(file);
		}
	};
	
	public static class AssetsSource extends FileSource {
		
		private AssetManager am;
		
		public AssetsSource(AssetManager a) {
			am = a;
		}
		
		String[] list(String dir) throws IOException {
			return am.list(dir);
		}
		
		InputStream open(String file) throws IOException {
			return am.open(file);
		}
		
	};
	
	public static String getContent(InputStream in) throws IOException {
		String retval = "";
		byte[] buf = new byte[512];
		while(true) {
			int n = in.read(buf);
			if (n <= 0)
				break;
			retval += new String(buf);
		}
		return retval;
	}
	
	public static boolean isContentsEquals(FileSource source1, String file1, FileSource source2, String file2) throws IOException {
		InputStream stream1 = null;
		InputStream stream2 = null;
		try {
			stream1 = source1.open(file1);
			stream2 = source2.open(file2);
			
			String newName = Utils.getContent(stream1);
			String oldName = Utils.getContent(stream2);
			return newName.equals(oldName);
		} catch (Exception e) {
			return false;
		}
		finally {
			if (stream1 != null) stream1.close();
			if (stream2 != null) stream2.close();
		}
	}
	
	public static void deleteRecursively(File target) throws IOException {
        if (!target.exists())	
            return;
        
		if (target.isDirectory()) {
			String[] children = target.list();
			for(int i = 0; i != children.length; ++i)
				deleteRecursively(new File(target, children[i]));
		}
		
	    //platformLog("delete", target.getPath());
		
		if (!target.delete())
			throw new IOException("Can not delete " + target.getAbsolutePath());
	}

	public static void deleteChildrenIgnoreFirstLevel(File target, String strIgnore) throws IOException {
        if (!target.exists())	
            return;
	
		if (target.isDirectory()) {
			String[] children = target.list();
			for(int i = 0; i != children.length; ++i)
			{
			    File f = new File(target, children[i]);
			    if ( f.isDirectory())
				    deleteRecursively(f);
				else if ( !f.getName().startsWith(strIgnore))    
				{
		            if (!f.delete())
			            throw new IOException("Can not delete " + f.getAbsolutePath());
				}
			}
		}
	}
	
	public static void copyRecursively(FileSource fs, File source, File target, boolean deleteTarget) throws IOException
	{
		if (deleteTarget && target.exists())
			deleteRecursively(target);
		
		if (source.isDirectory()) {
    		String[] children = fs.list(source.getAbsolutePath());
    		if (children != null && children.length > 0) {
    			if (!target.exists())
    				target.mkdirs();
    			
    			for(String child: children)
    				copyRecursively(fs, new File(source, child), new File(target, child), false);
    		}
		} else if (source.isFile()){
			InputStream in = null;
			OutputStream out = null;
			try {
				in = fs.open(source.getAbsolutePath());
				target.getParentFile().mkdirs();
				out = new FileOutputStream(target);
				
				byte[] buf = new byte[1024];
				int len;
				while((len = in.read(buf)) > 0)
					out.write(buf, 0, len);
				
			}
			catch (FileNotFoundException e) {
				if (in != null)
					throw e;
				
				target.createNewFile();
			}
			finally {
				if (in != null)
					in.close();
				if (out != null)
					out.close();
			}
		}
	}
	
	public static void copy(String src, String dst) throws IOException {
		InputStream is = null;
		OutputStream os = null;
		try {
			is = new FileInputStream(src);
			os = new FileOutputStream(dst);
			
			byte[] buf = new byte[1024];
			for(;;) {
				int n = is.read(buf);
				if (n <= 0)
					break;
				os.write(buf, 0, n);
			}
			os.flush();
		}
		finally {
			if (is != null)
				is.close();
			if (os != null)
				os.close();
		}
	}
	
	public static String getDirName(String filePath) {
		if (filePath == null)
			return null;
		return new File(filePath).getParent();
	}
	
	public static String getBaseName(String filePath) {
		if (filePath == null)
			return null;
		return new File(filePath).getName();
	}
	
	public static void platformLog(String tag, String message) {
		StringBuilder s = new StringBuilder();
		s.append("ms[");
		s.append(System.currentTimeMillis());
		s.append("] ");
		s.append(message);
		android.util.Log.v(tag, s.toString());
	}
	
}
