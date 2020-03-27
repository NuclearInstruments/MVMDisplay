
#include <lvgl.h>
#include <Ticker.h>
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"
#include <Adafruit_STMPE610.h>

//#include <TFT_eSPI.h>
//#include "lsetup.h"

#define LVGL_TICK_PERIOD 20




#define TFT_RST -1 // RST can be set to -1 if you tie it to Arduino's reset

   #define STMPE_CS 32
   #define TFT_CS   15
   #define TFT_DC   33
   #define SD_CS    14
   
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750

void lv_ex_btn_1(void);

Ticker tick; /* timer for interrupt handler */
//TFT_eSPI tft = TFT_eSPI(); /* TFT instance */
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

#if USE_LV_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char * file, uint32_t line, const char * dsc)
{

  Serial.printf("%s@%d->%s\r\n", file, line, dsc);
  delay(100);
}
#endif


bool my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
    uint16_t touchX, touchY;
    bool touched;
  if (ts.touched() < 1)
  {
    touched = false;
  }
  else
  {
    touched = true;
  }
   // bool touched = tft.getTouch(&touchX, &touchY, 600);
    TS_Point p = ts.getPoint();
    
   // Serial.print("X = "); Serial.print(p.x); Serial.print("\tY = "); Serial.print(p.y);  Serial.print("\tPressure = "); Serial.println(p.z); 
   
    // Scale from ~0->4000 to tft.width using the calibration #'s
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.height(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.width());

    touchX = p.y;
    touchY = tft.height() - p.x;


    if(touchY>tft.height() || touchX > tft.width())
    {
     /* Serial.println("Y or y outside of expected parameters..");
      Serial.print("y:");
      Serial.print(touchX);
      Serial.print(" x:");
      Serial.print(touchY);*/
        data->state = touched ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL; 
    }
    else
    {

      data->state = touched ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL; 
  
      /*Save the state and save the pressed coordinate*/
      //if(data->state == LV_INDEV_STATE_PR) touchpad_get_xy(&last_x, &last_y);
     
      /*Set the coordinates (if released use the last pressed coordinates)*/
      data->point.x = touchX;
      data->point.y = touchY;
  
      Serial.print("Data x");
      Serial.println(touchX);
      
      Serial.print("Data y");
      Serial.println(touchY);

    }

    return false; /*Return `false` because we are not buffering and no more data to read*/
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint16_t c;

  tft.startWrite(); /* Start new TFT transaction */
  tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      c = color_p->full;
      tft.writeColor(c, 1);
      color_p++;
    }
  }
  tft.endWrite(); /* terminate TFT transaction */
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

/* Interrupt driven periodic handler */
static void lv_tick_handler(void)
{

  lv_tick_inc(LVGL_TICK_PERIOD);
}

lv_obj_t *label;

void setup() {

  Serial.begin(115200); /* prepare for possible serial debug */

  
  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }
  Serial.println("Touchscreen started");
  
  lv_init();

#if USE_LV_LOG != 0
  lv_log_register_print(my_print); /* register print function for debugging */
#endif

  tft.begin(); /* TFT init */
  tft.setRotation(1); /* Landscape orientation */

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 480;
  disp_drv.ver_res = 320;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);


  /*Initialize the touch pad*/
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  /*Initialize the graphics library's tick*/
  tick.attach_ms(LVGL_TICK_PERIOD, lv_tick_handler);

  /* Create simple label */
 /* lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(label, "Hello Arduino! (V6.1)");
  lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
*/
  lv_ex_tabview_1();

}

static void event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        printf("Clicked\n");
    }
    else if(event == LV_EVENT_VALUE_CHANGED) {
        printf("Toggled\n");
    }
}

static void slider_event_cb_rr(lv_obj_t * slider, lv_event_t event);
static void slider_event_cb_rratio(lv_obj_t * slider, lv_event_t event);
static lv_obj_t * slider_label_rr;
static lv_obj_t * slider_label_rratio;

lv_style_t * getGreenStyle()
{
    static lv_style_t style;
    lv_style_copy(&style, &lv_style_plain_color);
    style.body.main_color = lv_color_hex3(0x0F0);     /*Line color at the beginning*/
    style.body.grad_color =  lv_color_hex3(0x0F0);    /*Line color at the end*/
   
    return &style;
}

void lv_ex_tabview_1(void)
{
    /*Create a Tab view object*/
    lv_obj_t *tabview;
    tabview = lv_tabview_create(lv_scr_act(), NULL);

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Live");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Plot");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Setup");

    lv_tabview_set_sliding(tabview,false);
    lv_page_set_sb_mode(tab1, LV_SB_MODE_OFF);
    lv_page_set_sb_mode(tab2, LV_SB_MODE_OFF);
    lv_page_set_sb_mode(tab3, LV_SB_MODE_OFF);
    

    static lv_style_t red_led;
    lv_style_copy(&red_led, &lv_style_pretty_color);
    red_led.body.radius = LV_RADIUS_CIRCLE;
    red_led.body.main_color = LV_COLOR_MAKE(0xb5, 0x0f, 0x04);
    red_led.body.grad_color = LV_COLOR_MAKE(0x50, 0x07, 0x02);
    red_led.body.border.color = LV_COLOR_MAKE(0xfa, 0x0f, 0x00);
    red_led.body.border.width = 3;
    red_led.body.border.opa = LV_OPA_30;
    red_led.body.shadow.color = LV_COLOR_MAKE(0xb5, 0x0f, 0x04);
    red_led.body.shadow.width = 5;

    static lv_style_t green_led;
    lv_style_copy(&green_led, &lv_style_pretty_color);
    green_led.body.radius = LV_RADIUS_CIRCLE;
    green_led.body.main_color = LV_COLOR_MAKE(0x00, 0xff, 0x00);
    green_led.body.grad_color = LV_COLOR_MAKE(0x00, 0xb5, 0x00);
    green_led.body.border.color = LV_COLOR_MAKE(0x00, 0xff, 0x00);
    green_led.body.border.width = 3;
    green_led.body.border.opa = LV_OPA_30;
    green_led.body.shadow.color = LV_COLOR_MAKE(0x00, 0x3f, 0x00);
    green_led.body.shadow.width = 5;

    lv_obj_t * led1  = lv_led_create(tab1, NULL);
    lv_obj_set_style(led1,  &red_led);
    lv_obj_align(led1, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 15);
    lv_led_off(led1);


    lv_obj_t * led2  = lv_led_create(tab1, NULL);
    lv_obj_set_style(led2,  &green_led);
    lv_obj_align(led2, NULL, LV_ALIGN_IN_TOP_LEFT, 60, 15);
    lv_led_on(led2);

    lv_obj_t * led3  = lv_led_create(tab1, NULL);
    lv_obj_set_style(led3,  &green_led);
    lv_obj_align(led3, NULL, LV_ALIGN_IN_TOP_LEFT, 240, 15);
    lv_led_on(led3);
    

    static lv_style_t style;
    lv_style_copy(&style, &lv_style_pretty_color);
    style.body.main_color = lv_color_hex3(0x666);     /*Line color at the beginning*/
    style.body.grad_color =  lv_color_hex3(0x666);    /*Line color at the end*/
    style.body.padding.left = 10;                      /*Scale line length*/
    style.body.padding.inner = 8 ;                    /*Scale label padding*/
    style.body.border.color = lv_color_hex3(0x333);   /*Needle middle circle color*/
    style.line.width = 3;
    style.text.color = lv_color_hex3(0x333);
    style.line.color = LV_COLOR_RED;                  /*Line color after the critical value*/

    /*Describe the color for the needles*/
    static lv_color_t needle_colors[3];
    needle_colors[0] = LV_COLOR_BLUE;
    needle_colors[1] = LV_COLOR_ORANGE;
    needle_colors[2] = LV_COLOR_PURPLE;

    /*Create a gauge*/
    lv_obj_t * gauge1 = lv_gauge_create(tab1, NULL);
    lv_gauge_set_style(gauge1, LV_GAUGE_STYLE_MAIN, &style);
    lv_gauge_set_needle_count(gauge1, 1, needle_colors);
    lv_obj_set_size(gauge1, 140, 140);
    lv_obj_align(gauge1, NULL, LV_ALIGN_IN_TOP_RIGHT, -20, 20);
    lv_gauge_set_range(gauge1,0,50);
    lv_gauge_set_value(gauge1, 0, 10);

    lv_obj_t * gauge2 = lv_gauge_create(tab1, NULL);
    lv_gauge_set_style(gauge2, LV_GAUGE_STYLE_MAIN, &style);
    lv_gauge_set_needle_count(gauge2, 1, needle_colors);
    lv_obj_set_size(gauge2, 140, 140);
    lv_obj_align(gauge2, NULL, LV_ALIGN_IN_TOP_RIGHT, -20, 130);
    lv_gauge_set_range(gauge2,0,150);
    lv_gauge_set_value(gauge2, 0, 10);

    lv_obj_t * rect1_rate;
    rect1_rate = lv_obj_create(tab1, NULL);
    lv_obj_set_size(rect1_rate, 280, 90);
    lv_style_t * box_green = getGreenStyle();
    lv_obj_set_style(rect1_rate, box_green);
    lv_obj_align(rect1_rate, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 55);
    lv_obj_t * label_rate = lv_label_create(rect1_rate, NULL);
    lv_obj_align(label_rate, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label_rate, "0 ppm");
    static lv_style_t st;
    lv_style_copy( &st, &lv_style_plain );
    //st.text.font = &lv_font_roboto_28;
    lv_obj_set_style( label_rate, &st );
   
    lv_obj_t * rect2_siop;
    rect2_siop = lv_obj_create(tab1, NULL);
    lv_obj_set_size(rect2_siop, 280, 90);
    lv_obj_set_style(rect2_siop, box_green);
    lv_obj_align(rect2_siop, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 155);
    lv_obj_t * label_siop = lv_label_create(rect2_siop, NULL);
    lv_obj_align(label_siop, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label_siop, "0");  
    lv_obj_set_style( label_siop, &st );
    /*Add content to the tabs*/
    

      /* Create a slider in the center of the display */
    lv_obj_t * slider_rr = lv_slider_create(tab3, NULL);
    lv_obj_set_width(slider_rr, 250);
    lv_obj_align(slider_rr, NULL, LV_ALIGN_IN_TOP_LEFT, 145, 5);
    lv_obj_set_event_cb(slider_rr, slider_event_cb_rr);
    lv_slider_set_range(slider_rr, 0, 100);
    
    /* Create a label below the slider */
    slider_label_rr = lv_label_create(tab3, NULL);
    lv_label_set_text(slider_label_rr, "0");
    //lv_obj_set_auto_realign(slider_label, true);
    lv_obj_align(slider_label_rr, NULL, LV_ALIGN_IN_TOP_LEFT, 410, 10);
    
    lv_obj_t * label_rr = lv_label_create(tab3, NULL);
    lv_label_set_text(label_rr, "Respiratory Rate");
    lv_obj_align(label_rr, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 10);



    lv_obj_t * slider_rratio = lv_slider_create(tab3, NULL);
    lv_obj_set_width(slider_rratio, 250);
    lv_obj_align(slider_rratio, NULL, LV_ALIGN_IN_TOP_LEFT, 145, 55);
    lv_obj_set_event_cb(slider_rratio, slider_event_cb_rratio);
    lv_slider_set_range(slider_rratio, 0, 100);
    
    /* Create a label below the slider */
    slider_label_rratio = lv_label_create(tab3, NULL);
    lv_label_set_text(slider_label_rratio, "0");
    //lv_obj_set_auto_realign(slider_label, true);
    lv_obj_align(slider_label_rratio, NULL, LV_ALIGN_IN_TOP_LEFT, 410, 60);
    
    lv_obj_t * label_rratio = lv_label_create(tab3, NULL);
    lv_label_set_text(label_rratio, "Respiratory Ratio");
    lv_obj_align(label_rratio, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 60);


    lv_obj_t * chart;
    chart = lv_chart_create(tab2, NULL);
    lv_obj_set_size(chart, 470, 270);
    lv_obj_align(chart, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_chart_set_type(chart, LV_CHART_TYPE_POINT | LV_CHART_TYPE_LINE);   /*Show lines and points too*/
    lv_chart_set_series_opa(chart, LV_OPA_70);                            /*Opacity of the data series*/
    lv_chart_set_series_width(chart, 4);                                  /*Line width and point radious*/

    lv_chart_set_range(chart, 0, 100);

    /*Add two data series*/
    lv_chart_series_t * ser1 = lv_chart_add_series(chart, LV_COLOR_RED);
    lv_chart_series_t * ser2 = lv_chart_add_series(chart, LV_COLOR_GREEN);

    /*Set the next points on 'dl1'*/
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 30);
    lv_chart_set_next(chart, ser1, 70);
    lv_chart_set_next(chart, ser1, 90);

    /*Directly set points on 'dl2'*/
    ser2->points[0] = 90;
    ser2->points[1] = 70;
    ser2->points[2] = 65;
    ser2->points[3] = 65;
    ser2->points[4] = 65;
    ser2->points[5] = 65;
    ser2->points[6] = 65;
    ser2->points[7] = 65;
    ser2->points[8] = 65;
    ser2->points[9] = 65;

    lv_chart_refresh(chart); /*Required after direct set*/
    
}


static void slider_event_cb_rr(lv_obj_t * slider, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        static char buf[4]; /* max 3 bytes for number plus 1 null terminating byte */
        snprintf(buf, 4, "%u", lv_slider_get_value(slider));
        lv_label_set_text(slider_label_rr, buf);
    }
}

static void slider_event_cb_rratio(lv_obj_t * slider, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        static char buf[4]; /* max 3 bytes for number plus 1 null terminating byte */
        snprintf(buf, 4, "%u", lv_slider_get_value(slider));
        lv_label_set_text(slider_label_rratio, buf);
    }
}

void lv_ex_btn_1(void)
{
    lv_obj_t * label;

    lv_obj_t * btn1 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(btn1, event_handler);
    lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, -40);

    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Button");

    lv_obj_t * btn2 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(btn2, event_handler);
    lv_obj_align(btn2, NULL, LV_ALIGN_CENTER, 0, 40);
    lv_btn_set_toggle(btn2, true);
    lv_btn_toggle(btn2);
    lv_btn_set_fit2(btn2, LV_FIT_NONE, LV_FIT_TIGHT);

    label = lv_label_create(btn2, NULL);
    lv_label_set_text(label, "Toggled");
}

void loop() {
 /* static int i;
  char cc[256];
  sprintf(cc, "V: %d", i++);
  lv_label_set_text(label, cc);*/
  lv_task_handler(); /* let the GUI do its work */
  delay(5);
}
