package com.seerslab.facelandmarktracking.utils;

import android.content.Context;
import android.util.Log;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * Created by ahnsohyun on 4/21/16.
 */
public class FileUtils {
    public static boolean copyAssetFile(Context context, String srcFileName, String dstFilePath, byte[] buffer) {
        InputStream is = null;
        FileOutputStream fos = null;
        try {
            is = context.getAssets().open(srcFileName);
            fos = new FileOutputStream(dstFilePath);
            //byte[] buffer = new byte[1024];
            int readBytes;
            while ((readBytes = is.read(buffer)) != -1) {
                fos.write(buffer, 0, readBytes);
            }
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        } finally {
            try {
                is.close();
                fos.close();
            } catch (IOException e) {
                Log.e("FileUtils", "Closing fails.");
                return false;
            } catch (NullPointerException e) {
                return false;
            }
        }
        return true;
    }
}
