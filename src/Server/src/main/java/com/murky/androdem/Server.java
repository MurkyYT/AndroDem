package com.murky.androdem;

import com.murky.androdem.SurfaceControl;
import android.graphics.Rect;
import android.media.MediaCodecInfo;
import android.os.BatteryManager;
import android.os.Build;
import android.os.IBinder;
import java.io.IOException;
import java.util.List;
import java.util.Map;
import java.util.Locale;
import java.io.*;
import android.net.wifi.*;
import android.content.*;
import android.app.*;
import java.lang.reflect.*;
import java.text.*;
import java.util.*;
public final class Server {

    private Server() {
    }
    public static void main(String... args) throws Exception {
        if(args.length == 0)
        {
            System.out.println("[Server.java] No command is entered.");
            return;
        }
        switch(args[0])
        {
            case "display":
                if(args.length < 2 || args.length > 2)
                {
                    System.out.println("[Server.java] The syntax of the command is incorrect.");
                    break;
                }
                IBinder d = SurfaceControl.getBuiltInDisplay();
                if (d == null) {
                    System.out.println("[Server.java] Couldn't get the display.");
                    break;
                }
                SurfaceControl.setDisplayPowerMode(d, Integer.parseInt(args[1]));
                System.out.println("[Server.java] Succesfully changed display mode to "+args[1]+".");
                break;
            case "restart-wifi":
                try{
                    Process su = Runtime.getRuntime().exec("svc wifi disable");
                    su.waitFor();
                    Process su2 = Runtime.getRuntime().exec("svc wifi enable");
                    su2.waitFor();
                }catch(IOException e){
                    System.out.println(e);
                    break;
                }
                catch(InterruptedException e){
                    System.out.println(e);
                    break;
                }
                catch(Exception e){
                    System.out.println(e);
                    break;
                }
                System.out.println("[Server.java] Succesfully restarted wifi.");
                break;
            case "shell":
                try{
                    String commandToExec = "";
                    for (int i=1; i<args.length; i++) 
                    { 
                        commandToExec += args[i]+" ";
                    }
                    Process su = Runtime.getRuntime().exec(commandToExec);
                    BufferedReader stdInput = new BufferedReader(new 
                    InputStreamReader(su.getInputStream()));

                    BufferedReader stdError = new BufferedReader(new 
                        InputStreamReader(su.getErrorStream()));
                    // Read the output from the command
                    System.out.println("[Server.java] Here is the standard output of the command:\n");
                    String s = null;
                    while ((s = stdInput.readLine()) != null) {
                        System.out.println(s);
                    }

                    // Read any errors from the attempted command
                    System.out.println("[Server.java] Here is the standard error of the command (if any):\n");
                    while ((s = stdError.readLine()) != null) {
                        System.out.println(s);
                    }
                    su.waitFor();
                }catch(IOException e){
                    System.out.println(e);
                    break;
                }
                catch(InterruptedException e){
                    System.out.println(e);
                    break;
                }
                catch(Exception e){
                    System.out.println(e);
                    break;
                }
                break;
            case "cleanup":
                try{
                    Process su = Runtime.getRuntime().exec("svc power stayon false");
                    su.waitFor();
                    IBinder display = SurfaceControl.getBuiltInDisplay();
                    if (display == null) {
                        System.out.println("[Server.java] Couldn't get the display.");
                        break;
                    }
                    SurfaceControl.setDisplayPowerMode(display, 2);
                    Process su2 = Runtime.getRuntime().exec("rm /data/local/tmp/classes.dex");
                    su2.waitFor();
                }
                catch(IOException e)
                {
                    System.out.println(e);
                    break;
                }
                catch(Exception e)
                {
                    System.out.println(e);
                    break;
                }
                System.out.println("[Server.java] Succesfully did the cleanup procces.");
                break;
            case "help":
                System.out.println("[Server.java]\n"+"AndroDem server help:\n"
                +"the available commands are:\n"
                +"help - show this menu\n"
                +"shell [COMMAND] - run shell commands in android nativly\n"
                +"display [MODE] - change display mode (0/1/2)\n"
                +"cleanup - return to default parameters and remove the dexed classes\n");
                break;
            default:
                System.out.println("[Server.java] Unknown command.");
                break;
        }
    }
}
