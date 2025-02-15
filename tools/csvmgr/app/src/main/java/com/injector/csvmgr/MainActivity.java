package com.injector.csvmgr;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.view.View;

import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.List;

import de.siegmar.fastcsv.writer.CsvAppender;
import de.siegmar.fastcsv.writer.CsvWriter;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

//        ProbeTable customTable = new ProbeTable();
//        customTable.addProduct(0x1234, 0x0001, CdcAcmSerialDriver.class);
//        customTable.addProduct(0x1234, 0x0002, CdcAcmSerialDriver.class);
//
//        UsbSerialProber prober = new UsbSerialProber(customTable);

//        UsbManager manager = (UsbManager) getSystemService(Context.USB_SERVICE);
//        List<UsbSerialDriver> availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(manager);
//        if (availableDrivers.isEmpty()) {
//            System.out.println("No USB devices were found!");
//        }
//        System.out.println(availableDrivers.toString());
//
//        // Open a connection to the first available driver.
//        UsbSerialDriver driver = availableDrivers.get(0);
//        UsbDeviceConnection connection = manager.openDevice(driver.getDevice());
//        if (connection == null) {
//            // add UsbManager.requestPermission(driver.getDevice(), ..) handling here
//            return;
//        }
//
//        UsbSerialPort port = driver.getPorts().get(0); // Most devices have just one port (port 0)
//        try {
//            port.open(connection);
//            port.setParameters(115200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);
//        } catch (IOException e) {
//            e.printStackTrace();
//        }







//        File file = new File("foo.csv");
//        CsvWriter csvWriter = new CsvWriter();
//
//        try {
//            CsvAppender csvAppender = csvWriter.append(file, StandardCharsets.UTF_8);
//            // header
//            csvAppender.appendLine("header1", "header2");
//
//            // 1st line in one operation
//            csvAppender.appendLine("value1", "value2");
//
//            // 2nd line in split operations
//            csvAppender.appendField("value3");
//            csvAppender.appendField("value4");
//            csvAppender.endLine();
//        } catch (Exception ex) {
//
//        }

    }

    public void goCaptureActivity(View view) {
        Intent intent = new Intent(this, CaptureActivity.class);
//        EditText editText = (EditText) findViewById(R.id.editText);
//        String message = editText.getText().toString();
//        intent.putExtra(EXTRA_MESSAGE, message);
        startActivity(intent);
    }



}
