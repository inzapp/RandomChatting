package com.ranchat.core;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import com.ranchat.util.Views;
import com.ranchat.util.pRes;

import javafx.application.Platform;

public class ChattingManager {
	private ExecutorService threadPool;
	private Runnable receiveTask;
	private final int BUFSIZE = 512;
	
	private ChattingManager() {
		threadPool = Executors.newSingleThreadExecutor();
		receiveTask = () -> {
			BufferedReader br = null;
			
			try {
				InputStream is = pRes.socket.getInputStream();
				InputStreamReader isr = new InputStreamReader(is, "euc-kr");
				br = new BufferedReader(isr);
			} catch (IOException e) {
				e.printStackTrace();
				return;
			}
			
			char[] buf;
			while(true) {
				buf = new char[BUFSIZE + 1];
				try {
					br.read(buf);
				} catch (IOException e) {
					break;
				}

				final char[] finalBuf = buf;
				Platform.runLater(() -> {
					Views.textArea.appendText(String.valueOf(finalBuf).trim() + "\n");	
				});
			}
			
			Platform.runLater(() -> Views.textArea.appendText("[상대방이 나갔습니다]\n"));
		};
	}
	
	private static class Singleton {
		static ChattingManager INSTANCE = new ChattingManager();
	}
	
	public static ChattingManager getInstance() {
		return Singleton.INSTANCE;
	}

	public void StartChatting() {
		threadPool.execute(receiveTask);
	}
	
	public void sendEvent() {
		String msg = Views.editText.getText();
		Views.editText.setText("");
		if(msg.equals("")) return;
		
		DataOutputStream dos = null;
		try {
			OutputStream os = pRes.socket.getOutputStream();
			dos = new DataOutputStream(os);
		} catch(Exception e) {
			return;
		}
		
		try {
			dos.write(msg.getBytes("euc-kr"));
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
