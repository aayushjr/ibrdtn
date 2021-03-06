/*
 * SoundFXManager.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
package de.tubs.ibr.dtn.dtalkie.service;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map.Entry;


import android.content.Context;
import android.media.AudioManager;
import android.media.SoundPool;
import android.util.Log;

public class SoundFXManager {
	
	private final static String TAG = "SoundFXManager";
	private HashMap<Integer, SoundPool> poolMap = new HashMap<Integer, SoundPool>();
	
	public void load(Context context, Sound s) {
		for (Entry<Integer, SoundPool> entry: poolMap.entrySet()) {
			try {
				int id = entry.getValue().load(context.getAssets().openFd(s.getFilename()), 1);
				s.setSoundId(entry.getKey(), id);
			} catch (IOException e) {
				Log.e(TAG, "sound loading failed.", e);
			}
		}
	}
	
    public void initialize(int streamType, int maxStreams) {
    	poolMap.put(streamType, new SoundPool(maxStreams, streamType, 0));
    }
    
    public void play(Context context, int streamType, Sound s) {
    	if (!poolMap.containsKey(streamType)) {
    		Log.e(TAG, "stream-type not initialized!");
    		return;
    	}
    	
    	SoundPool sp = poolMap.get(streamType);

        /* Updated: The next 4 lines calculate the current volume in a scale of 0.0 to 1.0 */
        AudioManager mgr = (AudioManager)context.getSystemService(Context.AUDIO_SERVICE);
        float streamVolumeCurrent = mgr.getStreamVolume(streamType);
        float streamVolumeMax = mgr.getStreamMaxVolume(streamType);
        
        float volume = streamVolumeCurrent / streamVolumeMax;
        
        /* Play the sound with the correct volume */
        sp.play(s.getSoundId(streamType), volume, volume, 1, 0, 1f);
    }
}
