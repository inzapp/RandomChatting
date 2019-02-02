package com.ranchat;

import java.net.URL;
import java.util.ResourceBundle;

import com.ranchat.core.ChattingManager;
import com.ranchat.core.SocketConnector;
import com.ranchat.util.Views;
import com.ranchat.util.pRes;

import javafx.application.Application;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.TextArea;
import javafx.scene.image.ImageView;
import javafx.scene.input.KeyCode;
import javafx.stage.Stage;

public class RandomChattingClient extends Application implements Initializable{
	
	@FXML
	private TextArea textArea, editText;
	
	@FXML
	private ImageView reConnectBt, sendBt;

	@Override
	public void initialize(URL location, ResourceBundle resources) {
		Views.textArea = this.textArea;
		Views.editText = this.editText;
		Views.reConnectBt = this.reConnectBt;
		Views.sendBt = this.sendBt;
		
		Views.textArea.setWrapText(true);
		
		Views.editText.setOnKeyPressed(event -> {
			if(event.getCode() == KeyCode.ENTER) {
				ChattingManager.getInstance().sendEvent();
			}
		});
		
		Views.sendBt.setOnMouseClicked(event -> ChattingManager.getInstance().sendEvent());
		Views.reConnectBt.setOnMouseClicked(event -> SocketConnector.getInstance().connect());
		
		SocketConnector.getInstance().rawConnect();
	}
	
	@Override
	public void start(Stage primaryStage) throws Exception {
		Parent fxml = FXMLLoader.load(getClass().getResource("ranchat.fxml"));
		Scene scene = new Scene(fxml);

		primaryStage.setScene(scene);
		primaryStage.setTitle(pRes.PROGRAM_TITLE);
		primaryStage.setResizable(false);
		primaryStage.setOnCloseRequest(event -> System.exit(0));
		primaryStage.show();
	}
	
	public static void main(String[] args)  throws Exception{
		launch(args);
	}
}
