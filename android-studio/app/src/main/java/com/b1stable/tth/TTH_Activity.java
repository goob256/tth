package com.b1stable.tth;

import java.io.File;
import java.io.FileFilter;
import java.util.Locale;
import java.nio.file.Path;
import java.nio.file.PathMatcher;
import java.nio.file.FileSystems;
import java.util.Vector;

import android.os.Bundle;
import android.os.Build;
import android.content.Context;
import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Color;
import android.net.Uri;
import android.os.Vibrator;
import android.os.VibrationEffect;
import android.util.Log;
import android.view.Surface;
import android.view.Gravity;

import org.libsdl.app.SDLActivity; 

import com.b1stable.tth.License_Viewer_Activity;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;
import com.google.android.gms.auth.api.signin.GoogleSignIn;
import com.google.android.gms.auth.api.signin.GoogleSignInAccount;
import com.google.android.gms.auth.api.signin.GoogleSignInClient;
import com.google.android.gms.auth.api.signin.GoogleSignInOptions;
import com.google.android.gms.common.api.ApiException;
import com.google.android.gms.games.AchievementsClient;
import com.google.android.gms.games.AnnotatedData;
import com.google.android.gms.games.EventsClient;
import com.google.android.gms.games.Games;
import com.google.android.gms.games.GamesClient;
import com.google.android.gms.games.LeaderboardsClient;
import com.google.android.gms.games.Player;
import com.google.android.gms.games.PlayersClient;
import com.google.android.gms.games.event.Event;
import com.google.android.gms.games.event.EventBuffer;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.android.gms.tasks.Task;
import com.google.android.gms.tasks.Task.*;

public class TTH_Activity extends SDLActivity
{
	final static int LICENSE_REQUEST = 1;

	native static void resume_after_showing_license();
	native static void resume_after_showing_achievements();
	native static void pause();
	native static void resume();

	// Client used to sign in with Google APIs
	private GoogleSignInClient mGoogleSignInClient;
	private AchievementsClient mAchievementsClient;

	// request codes we use when invoking an external activity
	private static final int RC_UNUSED = 5001;
	private static final int RC_SIGN_IN = 9001;

	// This is so the screen is never cleared pure black, only shim::black (r:35, g:30, b:60)
	static boolean paused = false;

	 private static final String TAG = "TTH";

	@Override
	public void onCreate(Bundle savedInstance)
	{
		super.onCreate(savedInstance);
		// Create the client used to sign in to Google services.
		mGoogleSignInClient = GoogleSignIn.getClient(this,
		new GoogleSignInOptions.Builder(GoogleSignInOptions.DEFAULT_GAMES_SIGN_IN).build());
	}

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data)
	{
    super.onActivityResult(requestCode, resultCode, data);
		if (requestCode == LICENSE_REQUEST) {
			if (data != null) {
				if (resultCode == RESULT_OK && data.getExtras().getString("MESSAGE").equals("OK")) {
					show_license_result = 0;
				}
				else if (resultCode == RESULT_CANCELED && data.getExtras().getString("MESSAGE").equals("FAIL")) {
					show_license_result = 1;
				}
				else {
					show_license_result = 1;
				}
			}
			else {
				show_license_result = 1;
			}

			resume_after_showing_license();
		}
	    if (requestCode == RC_SIGN_IN) {
	      Task<GoogleSignInAccount> task =
		  GoogleSignIn.getSignedInAccountFromIntent(data);

	      try {
		GoogleSignInAccount account = task.getResult(ApiException.class);
		onConnected(account);
	      } catch (ApiException apiException) {
		String message = apiException.getMessage();
		if (message == null || message.isEmpty()) {
		    message = "An error occurred!";
		}

		onDisconnected();

		new AlertDialog.Builder(this)
		    .setMessage(message)
		    .setNeutralButton(android.R.string.ok, null)
		    .show();
	      }
	    }
	}

	public void onStart() {
		super.onStart();
	}

	@Override
	public void onStop()
	{
		super.onStop();
		pause();
	}
	
	@Override
	public void onRestart()
	{
		super.onRestart();
		resume();
	}

	@Override
	public void onResume()
	{
		super.onResume();
		signInSilently();
	}

	@Override
	public void onPostResume()
	{
		super.onPostResume();
		paused = true;
	}

	public void logString(String s)
	{
		Log.d("TTH", s);
	}

	public String getAppdataDir()
	{
		return getFilesDir().getAbsolutePath();
	}
	
	public String getSDCardDir()
	{
		File f = getExternalFilesDir(null);
		if (f != null) {
			return f.getAbsolutePath();
		}
		else {
			return getFilesDir().getAbsolutePath();
		}
	}

	static int show_license_result;

	public void showLicense()
	{
		show_license_result = -1;
		Intent intent = new Intent(this, License_Viewer_Activity.class);
		startActivityForResult(intent, LICENSE_REQUEST);
	}

	public int getShowLicenseResult()
	{
		return show_license_result;
	}

	/*
	public void openURL(String url)
	{
		Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
		startActivity(intent);
	}
	*/

	public void rumble(int milliseconds)
	{
		Vibrator v = (Vibrator)getSystemService(Context.VIBRATOR_SERVICE);
		if (v != null && v.hasVibrator()) {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
				v.vibrate(VibrationEffect.createOneShot(milliseconds, VibrationEffect.DEFAULT_AMPLITUDE));
			}
			else {
				v.vibrate(milliseconds);
			}
		}
	}

	public boolean has_touchscreen()
	{
		return getPackageManager().hasSystemFeature("android.hardware.touchscreen");
	}

	public boolean has_vibrator()
	{
		Vibrator v = (Vibrator)getSystemService(Context.VIBRATOR_SERVICE);
		if (v != null) {
			return v.hasVibrator();
		}
		else {
			return false;
		}
	}

	public void start_draw()
	{
		if (paused) {
			paused = false;
		}
	}
	
	public String get_android_language()
	{
		return Locale.getDefault().getLanguage();
	}

	private static File[] list_dir_files = null;

	public void list_dir_start(String path)
	{
		try {
			int slash = path.lastIndexOf('/');
			final String glob = path.substring(slash+1).replace("*", ".*"); // +1 works even if not found (-1+1 == 0)
			String dir = path.substring(0, slash);
			File f = new File(dir);
			list_dir_files = f.listFiles(new FileFilter() {
				public boolean accept(File f)
				{
					try {
						if (f.getName().matches(glob)) {
							return true;
						}
						else {
							return false;
						}
					}
					catch (Exception e) {
						Log.d("TTH", "list_dir_start FileFilter throwing " + e.getMessage());
						return false;
					}
				}
			});
		}
		catch (Exception e) {
			list_dir_files = null;
			Log.d("TTH", "list_dir_start throwing " + e.getMessage());
		}
	}

	public String list_dir_next()
	{
		if (list_dir_files == null) {
			return "";
		}
		else if (list_dir_files.length == 0) {
			list_dir_files = null;
			return "";
		}
		else {
			File f = list_dir_files[0];
			String name = f.getName();
			if (list_dir_files.length == 1) {
				list_dir_files = null;
			}
			else {
				File[] new_list = new File[list_dir_files.length-1];
				for (int i = 1; i < list_dir_files.length; i++) {
					new_list[i-1] = list_dir_files[i];
				}
				list_dir_files = new_list;
			}
			return name;
		}
	}

	private static final String ARC_DEVICE_PATTERN = ".+_cheets|cheets_.+";

	public boolean is_chromebook()
	{
		// Google uses this, so should work?
		return Build.DEVICE != null && Build.DEVICE.matches(ARC_DEVICE_PATTERN);
	}

	private static final int RC_ACHIEVEMENT_UI = 9003;

	public boolean show_achievements()
	{
		if (mAchievementsClient == null) {
			return false;
		}
		mAchievementsClient.getAchievementsIntent().addOnSuccessListener(new OnSuccessListener<Intent>() {
			@Override
			public void onSuccess(Intent intent) {
				startActivityForResult(intent, RC_ACHIEVEMENT_UI);
				resume_after_showing_achievements();
			}
		}).addOnFailureListener(new OnFailureListener() {
			@Override
			public void onFailure(Exception e) {
				resume_after_showing_achievements();
			}
		}).addOnCompleteListener(new OnCompleteListener<Intent>() {
			@Override
			public void onComplete(Task<Intent> task) {
				resume_after_showing_achievements();
			}
		});

		return true;
	}
	
	public void achieve(String id)
	{
		if (mAchievementsClient != null) {
GamesClient gamesClient = Games.getGamesClient(this, GoogleSignIn.getLastSignedInAccount(this));
gamesClient.setViewForPopups(findViewById(android.R.id.content));
gamesClient.setGravityForPopups(Gravity.TOP | Gravity.CENTER_HORIZONTAL);
			mAchievementsClient.unlock(id);
		}
	}

	private boolean isSignedIn() {
		return GoogleSignIn.getLastSignedInAccount(this) != null;
	}

	private void signInSilently() {
		Log.d(TAG, "signInSilently()");

		mGoogleSignInClient.silentSignIn().addOnCompleteListener(this,
			new OnCompleteListener<GoogleSignInAccount>() {
				@Override
				public void onComplete(@NonNull Task<GoogleSignInAccount> task) {
					if (task.isSuccessful()) {
					Log.d(TAG, "signInSilently(): success");
					onConnected(task.getResult());
				} else {
					Log.d(TAG, "signInSilently(): failure", task.getException());
					onDisconnected();
				}
			}
		});
	}

	public void start_google_play_games_services() {
		GoogleSignInAccount account = GoogleSignIn.getLastSignedInAccount(this);

		if (account != null) {
			GamesClient gamesClient = Games.getGamesClient(this, account);
			if (gamesClient != null) {
				gamesClient.setViewForPopups(findViewById(android.R.id.content));
				gamesClient.setGravityForPopups(Gravity.TOP | Gravity.CENTER_HORIZONTAL);
			}
		}

		startActivityForResult(mGoogleSignInClient.getSignInIntent(), RC_SIGN_IN);
	}


  private void signOut() {
    Log.d(TAG, "signOut()");

    if (!isSignedIn()) {
      Log.w(TAG, "signOut() called, but was not signed in!");
      return;
    }

    mGoogleSignInClient.signOut().addOnCompleteListener(this,
        new OnCompleteListener<Void>() {
          @Override
          public void onComplete(@NonNull Task<Void> task) {
            boolean successful = task.isSuccessful();
            Log.d(TAG, "signOut(): " + (successful ? "success" : "failed"));

            onDisconnected();
          }
        });
  }

  private void handleException(Exception e, String details) {
    int status = 0;

    if (e instanceof ApiException) {
      ApiException apiException = (ApiException) e;
      status = apiException.getStatusCode();
    }

    String message = "An error occurred!";

    new AlertDialog.Builder(TTH_Activity.this)
        .setMessage(message)
        .setNeutralButton(android.R.string.ok, null)
        .show();
  }

  private void onConnected(GoogleSignInAccount googleSignInAccount) {
    Log.d(TAG, "onConnected(): connected to Google APIs");

    mAchievementsClient = Games.getAchievementsClient(this, googleSignInAccount);
  }

  private void onDisconnected() {
    Log.d(TAG, "onDisconnected()");

    mAchievementsClient = null;
  }
}
