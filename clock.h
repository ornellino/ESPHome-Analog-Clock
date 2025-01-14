
void update_clock_hands(AddressableLight &it, ESPTime time, Color clock_ring_colors[], int offset) {
    static boolean initialized;
    int num_leds = it.size();

    if (initialized == false) {
      std::fill_n(clock_ring_colors, it.size(), Color::BLACK);
      initialized = true;
    }

    if (!time.is_valid()) {
      return;
    }

    // calculate led indices
    int second_idx = (int) (time.second * (it.size() / 60.0));
    int minute_idx = (int) (time.minute * (it.size() / 60.0));
    // this idx would "jump" in big steps
    int hour_idx = (int) ((time.hour % 12) * (it.size() / 12.0));
    // this moves one led at a time
    int hour_inc_min_idx = hour_idx + (int) (((float) time.minute / 12.0) * (it.size() / 60.0));

    // the hour hand may be bigger than one led, so we use an array for that
    std::vector<int> hour_idx_vector;
    if (id(wide_hour_hand_enabled)) {
      hour_idx_vector.push_back((hour_inc_min_idx - 1 + it.size()) % (it.size()));
      hour_idx_vector.push_back(hour_inc_min_idx);
      hour_idx_vector.push_back((hour_inc_min_idx + 1) % it.size());
    } else {
      hour_idx_vector.push_back(hour_inc_min_idx);
    }

    // fade out old colors
    for (int i = 0; i < it.size(); i++) {
      Color old_color = clock_ring_colors[i];
      int percentage = 2;
      int red = old_color.red;
      int red_sub = std::max(1, (int) ((red / 100.0) * percentage));
      int green = old_color.green;
      int green_sub = std::max(1, (int) ((green / 100.0) * percentage));
      int blue = old_color.blue;
      int blue_sub = std::max(1, (int) ((blue / 100.0) * percentage));
      if (red <= red_sub) { red = 0; } else { red -= red_sub; }
      if (green <= green_sub) { green = 0; } else { green -= green_sub; }
      if (blue <= blue_sub) { blue = 0; } else { blue -= blue_sub; }
      Color new_color = Color(red, green, blue, 0);
      clock_ring_colors[i] = new_color;
    }

    int indicator_brightness = id(clock_brightness) / 2;
    // limit min brightness to 60
    if (indicator_brightness < 60) { indicator_brightness = 60; }
    Color indicator_color = Color::BLACK;
    if (id(auto_brightness_bg_enabled)) {
      indicator_color = Color(indicator_brightness, indicator_brightness, indicator_brightness);
    } else {
      indicator_color = Color(id(hrs_brightness), id(hrs_brightness), id(hrs_brightness));
    }

    // set base colors
    Color hour_color = Color(0, 0, id(clock_brightness));
    Color minute_color = Color(0, id(clock_brightness), 0);
    Color second_color = Color(id(clock_brightness), 0, 0);
    if (!id(clock_seconds_enabled)) {
      second_color = Color::BLACK;
    }

    // merge colors if they overlap
    // if minute idx inside hour idx range
    if (std::find(std::begin(hour_idx_vector), std::end(hour_idx_vector), minute_idx) != std::end(hour_idx_vector)) {
      minute_color += hour_color;
    }

    if (second_idx == minute_idx) {
      minute_color += second_color;
      second_color = minute_color;
    } else if (std::find(std::begin(hour_idx_vector), std::end(hour_idx_vector), second_idx) != std::end(hour_idx_vector)) {
      second_color += hour_color;
    }

    // === set colors from bottom to top
    // indicators
    if (id(clock_indicators_enabled)) {
      for (int i = 0; i < it.size(); i += (int) ((float) it.size() / 12)) {
        clock_ring_colors[i] = indicator_color;
      }
    }

    // hours
    for (int i = 0; i < hour_idx_vector.size(); i++) {
      clock_ring_colors[hour_idx_vector[i]] = hour_color;
    }
    // minutes
    clock_ring_colors[minute_idx] = minute_color;
    // seconds
    if (id(clock_seconds_enabled)) {
      clock_ring_colors[second_idx] = second_color;
    }

    // apply colors to light
    for (int i = 0; i < it.size(); i++) {
      int x = (i + offset) % it.size();
      it[x] = clock_ring_colors[i];
    }
}