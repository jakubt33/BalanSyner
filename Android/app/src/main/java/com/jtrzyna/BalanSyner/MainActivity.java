package com.jtrzyna.BalanSyner;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import java.nio.ByteBuffer;


public class MainActivity extends Activity {

    private final static String _DEV_NAME = "K";
    private final static int REQUEST_ENABLE_BT = 3;
    private final static int _MSG_TEXT_BUTTON_CONNECT = 5;
    private final static int SUCCESS_CONNECT = 6;
    private final static int ERROR = -1;
    private final static int TERMINATE = -2;
    private final static int _MAX_PROGRESS = 200;
    private final static int _TIME_INTERVAL = 50; //ms
    private final static byte _START = (byte)0xFF;
    private final static byte _ADDRESS_SPEED = 113;
    private final static byte _ADDRESS_ROTATION = 114;
    private final static byte _PARITY = 0x01;

    //UI
    ToggleButton buttonConnect;

    TextView textSpeed;
    TextView textRotation;

    SeekBar seekSpeed;
    SeekBar seekRotation;

    private BtManager btManager;

    //Global variables
    private float speed=0;
    private float rotation=0;
    private static boolean DISCOVERY_STARTED = false;
    private static boolean foundBTTarget = false;

    BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

    private Runnable timeRunnable = new Runnable() {
        @Override
        public void run() {

            textSpeed.setText(String.valueOf(speed));
            byte[] valueSpeed = ByteBuffer.allocate(4).putFloat(speed).array();
            int[] message = new int[16];
            message[0] = _START;
            message[1] = _START;
            message[2] = _ADDRESS_SPEED;
            message[3] = valueSpeed[3];
            message[4] = valueSpeed[2];
            message[5] = valueSpeed[1];
            message[6] = valueSpeed[0];
            message[7] = _PARITY;

            textRotation.setText(String.valueOf(rotation));
            byte[] valueRotation = ByteBuffer.allocate(4).putFloat(rotation).array();

            message[8] = _START;
            message[9] = _START;
            message[10] = _ADDRESS_ROTATION;
            message[11] = valueRotation[3];
            message[12] = valueRotation[2];
            message[13] = valueRotation[1];
            message[14] = valueRotation[0];
            message[15] = _PARITY;
            btManager.sendMessage(message);

            /* and here comes the "trick" */
            timeHandler.postDelayed(this, _TIME_INTERVAL);
        }
    };

    private Handler timeHandler = new Handler();


    Handler mainHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case SUCCESS_CONNECT:
                    buttonConnect.setChecked(true);
                    timeHandler.postDelayed(timeRunnable, _TIME_INTERVAL);
                    break;
                case _MSG_TEXT_BUTTON_CONNECT:
                    buttonConnect.setText(msg.obj.toString());
                    break;
                case ERROR:
                    Log.d("here comes", "THE ERROR");
                    switch (msg.arg1) {
                        case TERMINATE:
                            Toast.makeText(getApplicationContext(), "No bluetooth detected", Toast.LENGTH_SHORT).show();
                            finish();
                            break;
                    }
                    //deactivateSystem();
                    break;
                default:
                    break;
            }
        }
    };

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (BluetoothAdapter.ACTION_DISCOVERY_STARTED.equals(action)) {
                if (DISCOVERY_STARTED) {
                    mainHandler.obtainMessage(_MSG_TEXT_BUTTON_CONNECT, "searching...").sendToTarget();
                    foundBTTarget = false;
                }
            } else if (BluetoothDevice.ACTION_FOUND.equals(action)) {
                // Get the BluetoothDevice object from the Intent
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                // Add the name and address to an array adapter to show in a ListView

                if (device.getName().equals(_DEV_NAME)) {//Unique device address
                    if (DISCOVERY_STARTED && (!foundBTTarget)) {
                        mainHandler.obtainMessage(_MSG_TEXT_BUTTON_CONNECT, "connecting...").sendToTarget();
                        foundBTTarget = true;
                        mBluetoothAdapter.cancelDiscovery();
                        btManager.connect(device);
                    }
                }
            } else if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(action)) {
                if ((!foundBTTarget) && DISCOVERY_STARTED) {
                    buttonConnect.setChecked(false);
                    Toast.makeText(getApplicationContext(), "Nie znaleziono urządzenia bluetooth", Toast.LENGTH_SHORT).show();
                }
                DISCOVERY_STARTED = false;

            } else if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(action)) {
                final int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);
                switch (state) {
                    case BluetoothAdapter.STATE_OFF:
                        buttonConnect.setChecked(false);
                        Toast.makeText(getApplicationContext(), "Musisz włączyć bluetooth aby kontunuować", Toast.LENGTH_SHORT).show();
                        deactivate();
                        break;
                    case BluetoothAdapter.STATE_TURNING_OFF:
                        mainHandler.obtainMessage(_MSG_TEXT_BUTTON_CONNECT, "Turning off...").sendToTarget();
                        break;
                    case BluetoothAdapter.STATE_ON:
                        mainHandler.obtainMessage(_MSG_TEXT_BUTTON_CONNECT, "Connect").sendToTarget();
                        Log.d("state", "STATE ON");
                        if (DISCOVERY_STARTED) {
                            Log.d("state", "STATE ON & DIS START == true");
                            mBluetoothAdapter.startDiscovery();
                        }
                        break;
                    case BluetoothAdapter.STATE_TURNING_ON:
                        Log.d("state", "TURNING ON");
                        mainHandler.obtainMessage(_MSG_TEXT_BUTTON_CONNECT, "Turning on...").sendToTarget();
                        break;
                }
            } else if (BluetoothDevice.ACTION_ACL_DISCONNECTED.equals(action)) {
                Toast.makeText(getApplicationContext(), "Utracono połączenie", Toast.LENGTH_SHORT).show();
                Log.d("state", "dev disconnected");
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initButtons();
        initTextViews();
        initSeekBars();
        initBt();
        registerIntentFilters();
    }

    void initBt() {
        checkBluetoothAdapter();
        btManager = new BtManager(mainHandler); //handler to communicate, name to find
    }

    void initButtons() {
        buttonConnect = (ToggleButton) findViewById(R.id.button_connect);
    }

    void initSeekBars(){
        seekSpeed = (SeekBar) findViewById(R.id.SeekSpeed);

        seekSpeed.setMax(_MAX_PROGRESS);
        seekSpeed.setProgress(0); // Set initial progress value

        seekSpeed.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean b) {
                speed = progress-_MAX_PROGRESS/2;
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) { }
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                speed =0;
            }
        });

        seekRotation = (SeekBar) findViewById(R.id.SeekRotation);

        seekRotation.setMax(_MAX_PROGRESS);
        seekRotation.setProgress(0); // Set initial progress value

        seekRotation.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean b) {
                rotation = progress-_MAX_PROGRESS/2;
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) { }
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                rotation = 0;
            }
        });
    }

    void initTextViews(){
        textSpeed = (TextView) findViewById(R.id.TextSpeed);
        textRotation = (TextView) findViewById(R.id.TextRotation);
    }


    private void registerIntentFilters() {
        IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
        registerReceiver(mReceiver, filter);
        filter = new IntentFilter(BluetoothDevice.ACTION_FOUND);
        registerReceiver(mReceiver, filter);
        filter = new IntentFilter(BluetoothAdapter.ACTION_DISCOVERY_STARTED);
        registerReceiver(mReceiver, filter);
        filter = new IntentFilter(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        registerReceiver(mReceiver, filter);
    }

    private void checkBluetoothAdapter() {
        if (mBluetoothAdapter == null) {
            Toast.makeText(getApplicationContext(), "No bluetooth detected", Toast.LENGTH_SHORT).show();
            finish();
            // Device does not support Bluetooth
        }
    }

    private void deactivate() {

        if (DISCOVERY_STARTED) {
            mBluetoothAdapter.cancelDiscovery();
        }
        disconnect();
    }

    private void disconnect() {
        timeHandler.removeCallbacks(timeRunnable);
        DISCOVERY_STARTED = false;
        foundBTTarget = false;
        btManager.disconnect();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    public void onButtonConnectClicked(View view) {
        boolean on = buttonConnect.isChecked();
        if (on) {
            DISCOVERY_STARTED = true;
            if (!mBluetoothAdapter.isEnabled()) {
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            } else {
                mBluetoothAdapter.startDiscovery();
            }
        } else {
            disconnect();
        }
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == REQUEST_ENABLE_BT) {
            if (resultCode == RESULT_CANCELED) {
                buttonConnect.setChecked(false);
                Toast.makeText(getApplicationContext(), "Wystąpił błąd przy włączaniu bluetootha", Toast.LENGTH_SHORT).show();
            }
            /*
            else if(resultCode == RESULT_OK){
                //if ok - BroadcastReceiver can handle it
            }*/
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        // Unregister broadcast listeners
        unregisterReceiver(mReceiver);
        deactivate();
    }
}