package com.ranchat.core;

import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.Optional;

import com.ranchat.util.Views;
import com.ranchat.util.pRes;

import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.ButtonType;

public class SocketConnector {
	private SocketConnector() {}
	
	private static class Singleton {
		static SocketConnector INSTANCE = new SocketConnector();
	}
	
	public static SocketConnector getInstance() {
		return Singleton.INSTANCE;
	}
	
	public void connect() {
		if(!alert()) return;

		disconnect();
		rawConnect();
	}
	
	public void rawConnect() {
		pRes.socket = new Socket();
		InetSocketAddress endPoint = new InetSocketAddress(pRes.IP, pRes.PORT);
		try {
			pRes.socket.connect(endPoint, pRes.TIMEOUT);
		} catch (Exception e) {
			Views.textArea.appendText("[서버와 연결이 불안정합니다]\n");
			e.printStackTrace();
			return;
		}
		
		Views.textArea.setText("");
		ChattingManager.getInstance().StartChatting();
	}
	
	private boolean alert() {
		Alert alert = new Alert(AlertType.CONFIRMATION);
		alert.setTitle(pRes.PROGRAM_TITLE);
		alert.setHeaderText(null);
		alert.setContentText("대화중인 상대와 연결이 끊어집니다\n새로 연결하시겠습니까?");
		
		ButtonType yes = new ButtonType("예");
		ButtonType no = new ButtonType("아니오");
		
		alert.getButtonTypes().setAll(yes, no);
		Optional<ButtonType> result = alert.showAndWait();
		
		if(result.get() == yes) return true;
		return false;
	}
	
	private void disconnect() {
		try {
			OutputStream os = pRes.socket.getOutputStream();
			os.close();
		} catch (Exception ignored) {
		}
	}
}
