#include <zephyr/types.h> //includes a type of library
#include <stddef.h>	  //includes a type of library
#include <string.h>	  //includes a type of library
#include <errno.h> 	  //includes a type of library
#include <sys/printk.h>	  //includes a type of library
#include <sys/byteorder.h>//includes a type of library
#include <zephyr.h>	  //includes a type of library

#include <settings/settings.h>	//includes a type of library

#include <bluetooth/bluetooth.h>//includes a type of library
#include <bluetooth/hci.h>	//includes a type of library
#include <bluetooth/conn.h> 	//includes a type of library
#include <bluetooth/uuid.h>	//includes a type of library
#include <bluetooth/gatt.h>	//includes a type of library
#include <device.h>		//includes a type of library
#include <drivers/sensor.h>	//includes a type of library

#include <stdio.h>		//includes a type of library
#include <stdint.h>
		
#include "matrix.h" 		//includes matrix.h file in the directory assignment
#include "buttons.h"		//includes a buttons.h file in the directory assignment

#define BT_UUID_BUTTON_SERVICE_VAL 	BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x2000)		//defines the bluetooth identifier
#define BT_UUID_BUTTON_A_ID  	   	BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x2001)		//defines the bluetooth identifier
#define BT_UUID_BUTTON_B_ID  	   	BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x2002)		//defines the bluetooth identifier

#define BT_UUID_COUNTER_SERVICE_VAL	BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x4000)
#define BT_UUID_UNLOCK_COUNTER_ID  	BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x4001)		//defines the bluetooth identifier
#define BT_UUID_LOCK_COUNTER_ID  	BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x4002)

#define BT_UUID_CODE_SERVICE_VAL	BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x5000)
#define BT_UUID_UNLOCK_CODE_ID  	BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x5001)		//defines the bluetooth identifier
#define BT_UUID_LOCK_CODE_ID	  	BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x5002)

static struct bt_uuid_128 my_button_uuid = BT_UUID_INIT_128(BT_UUID_BUTTON_SERVICE_VAL);	// the 128 bit UUID for this gatt value
static struct bt_uuid_128 button_a_id=BT_UUID_INIT_128(BT_UUID_BUTTON_A_ID); 			// the 128 bit UUID for this gatt value
static struct bt_uuid_128 button_b_id=BT_UUID_INIT_128(BT_UUID_BUTTON_B_ID); 			// the 128 bit UUID for this gatt value

static struct bt_uuid_128 my_counter_uuid = BT_UUID_INIT_128(BT_UUID_COUNTER_SERVICE_VAL); 
static struct bt_uuid_128 unlock_counter_id=BT_UUID_INIT_128(BT_UUID_UNLOCK_COUNTER_ID);
static struct bt_uuid_128 lock_counter_id=BT_UUID_INIT_128(BT_UUID_LOCK_COUNTER_ID);

static struct bt_uuid_128 my_code_uuid = BT_UUID_INIT_128(BT_UUID_CODE_SERVICE_VAL); 
static struct bt_uuid_128 unlock_code_id=BT_UUID_INIT_128(BT_UUID_UNLOCK_CODE_ID);
static struct bt_uuid_128 lock_code_id=BT_UUID_INIT_128(BT_UUID_LOCK_CODE_ID);


uint32_t button_a;		// the gatt characateristic value that is being shared over BLE 
uint32_t button_b;		// the gatt characateristic value that is being shared over BLE 
uint32_t button_a_state=0;	// the gatt characateristic value that is being shared over BLE  
uint32_t button_b_state=0;	// the gatt characateristic value that is being shared over BLE

uint32_t unlock_counter;	// the gatt characateristic value that is being shared over BLE
uint32_t unlock_counter_value=0;// the gatt characateristic value that is being shared over BLE

uint32_t lock_counter;		// the gatt characateristic value that is being shared over BLE
uint32_t lock_counter_value=0;	// the gatt characateristic value that is being shared over BLE

uint32_t unlock_code;		// the gatt characateristic value that is being shared over BLE
uint32_t unlock_code_value=0;	// the gatt characateristic value that is being shared over BLE

uint32_t lock_code;		// the gatt characateristic value that is being shared over BLE
uint32_t lock_code_value=0;	// the gatt characateristic value that is being shared over BLE

struct bt_conn *active_conn=NULL; // use this to maintain a reference to the connection with the central device (if any)

static ssize_t read_button_a(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
//callback that is activated when the characteristic is read by central
static ssize_t read_button_b(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
//callback that is activated when the characteristic is read by central
static ssize_t read_unlock_counter(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
//callback that is activated when the characteristic is read by central
static ssize_t read_lock_counter(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
//callback that is activated when the characteristic is read by central
static ssize_t read_unlock_code(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
//callback that is activated when the characteristic is read by central
static ssize_t read_lock_code(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
//callback that is activated when the characteristic is read by central


BT_GATT_SERVICE_DEFINE(my_button_svc,				//creates a new service for button
	BT_GATT_PRIMARY_SERVICE(&my_button_uuid),
	
		BT_GATT_CHARACTERISTIC(&button_a_id.uuid,		
		BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_READ,
		read_button_a, NULL, &button_a_state),		//enables the read feature
		
		BT_GATT_CHARACTERISTIC(&button_b_id.uuid,		
		BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_READ,
		read_button_b, NULL, &button_b_state)		//enables the read feature
		
);

BT_GATT_SERVICE_DEFINE(my_counter_svc,			//creates a new service for LED
	BT_GATT_PRIMARY_SERVICE(&my_counter_uuid),
	
		BT_GATT_CHARACTERISTIC(&unlock_counter_id.uuid,		
		BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
		BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
		read_unlock_counter, NULL, &unlock_counter_value),
		
		BT_GATT_CHARACTERISTIC(&lock_counter_id.uuid,		
		BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_READ,
		read_lock_counter, NULL, &lock_counter_value)		//enables the read feature		
);

BT_GATT_SERVICE_DEFINE(my_code_svc,			//creates a new service for LED
	BT_GATT_PRIMARY_SERVICE(&my_code_uuid),
	
		BT_GATT_CHARACTERISTIC(&unlock_code_id.uuid,		
		BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
		BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
		read_unlock_code, NULL, &unlock_code_value),
		
		BT_GATT_CHARACTERISTIC(&lock_code_id.uuid,		
		BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_READ,
		read_lock_code, NULL, &lock_code_value)		//enables the read feature		
);
static ssize_t read_button_a(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a read on button A state\n");	//prints on commandline when button A is pressed
	const char *value = (const char *)&button_a_state;// point at the value in memory
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(button_a_state));
	return 0;
}
static ssize_t read_button_b(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a read on button B state\n");	//prints on commandline when button B is pressed
	const char *value = (const char *)&button_b_state;// point at the value in memory
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(button_b_state));
	return 0;
}
static ssize_t read_unlock_counter(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a read for the unlock counter: %d\n", unlock_counter_value);	//prints on commandline when button B is pressed
	const char *value = (const char *)&unlock_counter_value;// point at the value in memory
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(unlock_counter_value));
	return 0;
}
static ssize_t read_lock_counter(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a read for the lock counter: %d\n", lock_counter_value);	//prints on commandline when button B is pressed
	const char *value = (const char *)&lock_counter_value;// point at the value in memory
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(lock_counter_value));
	return 0;
}
static ssize_t read_unlock_code(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a read for the unlock code: %d\n", unlock_code_value);	//prints on commandline when button B is pressed
	const char *value = (const char *)&unlock_code_value;// point at the value in memory
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(unlock_code_value));
	return 0;
}
static ssize_t read_lock_code(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a read for the lock code: %d\n", lock_code_value);	//prints on commandline when button B is pressed
	const char *value = (const char *)&lock_code_value;// point at the value in memory
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(lock_code_value));
	return 0;
}

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),	/* specify BLE advertising flags = discoverable, BR/EDR not supported (BLE only) */
        BT_DATA_BYTES(BT_DATA_UUID128_SOME, BT_UUID_BUTTON_SERVICE_VAL),		/* A 128 Service UUID for the our custom service follows */
        //BT_DATA_BYTES(BT_DATA_UUID128_SOME, BT_UUID_COUNTER_SERVICE_VAL),	/* A 128 Service UUID for the our custom service follows */
        //BT_DATA_BYTES(BT_DATA_UUID128_SOME, BT_UUID_CODE_SERVICE_VAL)		/* A 128 Service UUID for the our custom service follows */
};



static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printf("Connection failed (err 0x%02x)\n", err); //prints if the connection fails
	} else {
		printf("Connected\n");
		active_conn = conn;				//puts conn into active_conn when connected
	}
}
// Callback that is activated when a connection with a central device is taken down
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);
	active_conn = NULL;					//puts NULL into active_conn when disconnected
}
// structure used to pass connection callback handlers to the BLE stack
static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};
// This is called when the BLE stack has finished initializing
static void bt_ready(void)
{
	int err; 				//local veriable called err
	printf("Bluetooth initialized\n"); 	//prints when br_ready function is called
	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printf("Advertising failed to start (err %d)\n", err);
		return;
	} //prints if there a fault for advertising
	printf("Advertising successfully started\n");
}



void main(void)				//main
{	

	
	int err;			//new variable err
	int unlock_counter = 0;
	int lock_counter = 0;
	uint8_t rows = 1;		//initilsing rows for led matrix 
	uint8_t cols = 1;		//initilsing cols for led matrix
		
	uint32_t unlock_randnum;
	uint32_t lock_randnum;
	
	err = buttons_begin();	//calling buttons begin function in buttons.c
	if (err < 0)
	{
		printf("\nError initializing buttons.  Error code = %d\n",err);	
	 while(1);
	} //error checking for buttons
	err = matrix_begin();
	
	if (err < 0)
	{
		printf("\nError initializing buttons.  Error code = %d\n",err);	
	 while(1);
	}//error checking for led matrix
	err = bt_enable(NULL);
	
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}	//error checking for bluetooth
	bt_ready(); // This function starts advertising
	bt_conn_cb_register(&conn_callbacks);
	printf("Zephyr Microbit V2 minimal BLE example! %s\n", CONFIG_BOARD);			
	while (1) {
	
	//k_sleep(K_SECONDS(0.2));				//sleeps for 1 second
	matrix_all_off();
	if(get_buttonA() == 0){
			
		printf("\nButton A pressed!\n");	// prints when button A has been pressed
		unlock_counter++;
		unlock_counter_value = unlock_counter;
		printf("Unlock Counter Value: %d\n", unlock_counter);
		unlock_randnum = unlock_prbs();
		printf("Unlock Code: %x\n",unlock_randnum);
		unlock_code_value = unlock_randnum;
		
		if (active_conn) {
			bt_gatt_notify(active_conn,&my_button_svc.attrs[2], &button_a_state,sizeof(button_a_state));
		}	
		
		
		//displaying led pattern
		rows =  0b11111;		// sets the rows to all on		
   		cols =  0b10000; 		// sets the cols to all off
   		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
   		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b01000; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b00100; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b00010; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b00001; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b00010; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b00100; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b01000; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b10000; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b01000; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b00100; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b00010; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b00001; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b00010; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b00100; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b01000; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b11111;		// sets the rows to all on		
		cols =  0b10000; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));

		matrix_all_off();
		k_sleep(K_SECONDS(1));          		
	}
	
	if(get_buttonB() == 0)			
	{
		
		printf("\nButton B pressed!\n");	//prints when button B has been pressed
		printf("Lock Counter Value: %d\n", lock_counter);
		lock_counter++;
		lock_counter_value = lock_counter - 1;
		lock_randnum = lock_prbs();
		printf("Unlock Code: %x\n",lock_randnum);
		lock_code_value = lock_randnum;
		
		if (active_conn) {
			bt_gatt_notify(active_conn,&my_button_svc.attrs[4], &button_b_state,sizeof(button_b_state));
		}	
		
		rows =  0b10000;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b01000;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b00100;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b00010;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b00001;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b00010;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b00100;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b01000;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b10000;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b01000;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b00100;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b00010;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b00001;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b00010;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b00100;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b01000;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));
		
		rows =  0b10000;		// sets the rows to all on		
		cols =  0b11111; 		// sets the cols to all off
		matrix_put_pattern(rows, ~cols);// put the pattern on the matrix (tilda, ~ inverts cols)
		k_sleep(K_SECONDS(0.05));

		matrix_all_off();
		k_sleep(K_SECONDS(1));
	}
	
						
		
			
	}//end of while 1
}//end of main
