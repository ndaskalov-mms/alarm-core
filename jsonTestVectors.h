#pragma once
// This array contains the test vectors for the JSON MQTT commands
JsonMQTTTestVector jsonTestVectors[] = {
    //// Zone control tests
    //{"/my_alarm/config",
    // jsonBuffer,
    // "Test config JSON"},

     // Zone control tests
     {"/alarm/zones/control",
        "{\"zName\":\"Front Door\",\"bypass\":true}",
        "Bypass Front Door zone"},

     {"/alarm/partitions/control",
        "{\"pName\":\"Upper Floor\",\"arm_method\":\"arm_stay\"}",
        "Stay arm Garage partition"},

     //{"/my_alarm/zones/control",
     // "{\"zone\":\"Back Door\",\"action\":\"clear_bypass\"}",
     // "Clear bypass on Back Door zone"},

     //{"/my_alarm/zones/control",
     // "{\"zone\":\"Motion Sensor\",\"action\":\"tamper\"}",
     // "Trigger tamper on Motion Sensor zone"},

     //{"/my_alarm/zones/control",
     // "{\"zone\":\"Window Sensor\",\"action\":\"open\"}",
     // "Open Window Sensor zone"},

     //{"/my_alarm/zones/control",
     // "{\"zone\":\"Smoke Detector\",\"action\":\"close\"}",
     // "Close Smoke Detector zone"},

     //{"/my_alarm/zones/control",
     // "{\"zone\":\"PIR Living Room\",\"action\":\"anti-mask\"}",
     // "Trigger anti-mask on PIR Living Room zone"},

     // // Invalid zone tests
     // {"/my_alarm/zones/control",
     //  "{\"zone\":\"InvalidZone\",\"action\":\"bypass\"}",
     //  "Bypass an invalid zone"},

     // {"/my_alarm/zones/control",
     //  "{\"zone\":\"Front Door\",\"action\":\"invalid-action\"}",
     //  "Invalid action for a valid zone"},

     // {"/my_alarm/zones/control",
     //  "{\"invalid-key\":\"Front Door\",\"action\":\"bypass\"}",
     //  "Invalid JSON key for zone control"},

     // {"/my_alarm/zones/control",
     //  "{\"zone\":\"Front Door\"}",
     //  "Missing action key in JSON"},

     // {"/my_alarm/zones/control",
     //  "{\"action\":\"bypass\"}",
     //  "Missing zone key in JSON"},

     //  // Partition control tests
     //  {"/my_alarm/partitions/control",
     //   "{\"partition\":\"Main Floor\",\"action\":\"disarm\"}",
     //   "Disarm Main Floor partition"},

     //  {"/my_alarm/partitions/control",
     //   "{\"partition\":\"Upper Floor\",\"action\":\"arm\"}",
     //   "Arm Upper Floor partition"},

     //  {"/my_alarm/partitions/control",
     //   "{\"partition\":\"Basement\",\"action\":\"arm_force\"}",
     //   "Force arm Basement partition"},

     //  {"/my_alarm/partitions/control",
     //   "{\"partition\":\"Garage\",\"action\":\"arm_stay\"}",
     //   "Stay arm Garage partition"},

     //  {"/my_alarm/partitions/control",
     //   "{\"partition\":\"Office\",\"action\":\"arm_instant\"}",
     //   "Instant arm Office partition"},

     //   // Invalid partition tests
     //   {"/my_alarm/partitions/control",
     //    "{\"partition\":\"InvalidPartition\",\"action\":\"arm\"}",
     //    "Arm an invalid partition"},

     //   {"/my_alarm/partitions/control",
     //    "{\"partition\":\"Main Floor\",\"action\":\"invalid-action\"}",
     //    "Invalid action for a valid partition"},

     //    // PGM control tests
     //    {"/my_alarm/pgms/control",
     //     "{\"pgm\":\"Siren\",\"action\":\"on\"}",
     //     "Turn on Siren PGM"},

     //    {"/my_alarm/pgms/control",
     //     "{\"pgm\":\"Garage Door\",\"action\":\"off\"}",
     //     "Turn off Garage Door PGM"},

     //    {"/my_alarm/pgms/control",
     //     "{\"pgm\":\"Outdoor Lights\",\"action\":\"pulse\"}",
     //     "Pulse Outdoor Lights PGM"},

     //     // Invalid PGM tests
     //     {"/my_alarm/pgms/control",
     //      "{\"pgm\":\"InvalidPGM\",\"action\":\"on\"}",
     //      "Turn on an invalid PGM"},

     //     {"/my_alarm/pgms/control",
     //      "{\"pgm\":\"Siren\",\"action\":\"invalid-action\"}",
     //      "Invalid action for a valid PGM"},

     //      // Global options tests
     //      {"/my_alarm/global/options",
     //       "{\"option\":\"MaxSlaves\",\"value\":\"5\"}",
     //       "Set MaxSlaves option to 5"},

     //      {"/my_alarm/global/options",
     //       "{\"option\":\"RestrSprvsL\",\"value\":\"true\"}",
     //       "Enable restriction on supervision loss"},

     //      {"/my_alarm/global/options",
     //       "{\"option\":\"RestrTamper\",\"value\":\"false\"}",
     //       "Disable restriction on tamper"},

     //      {"/my_alarm/global/options",
     //       "{\"option\":\"RestrACfail\",\"value\":\"true\"}",
     //       "Enable restriction on AC failure"},

     //       // Invalid global options tests
     //       {"/my_alarm/global/options",
     //        "{\"option\":\"InvalidOption\",\"value\":\"true\"}",
     //        "Set an invalid global option"},

     //       {"/my_alarm/global/options",
     //        "{\"option\":\"MaxSlaves\",\"value\":\"invalid\"}",
     //        "Set MaxSlaves to an invalid value"},

     //        // Malformed JSON tests
     //        {"/my_alarm/zones/control",
     //         "{\"zone\":\"Front Door\",\"action\":\"bypass\"",
     //         "Malformed JSON - missing closing brace"},

     //        {"/my_alarm/partitions/control",
     //         "This is not JSON",
     //         "Invalid JSON format"},

     //        {"/my_alarm/pgms/control",
     //         "",
     //         "Empty payload"},

     //         // Empty JSON tests
     //         {"/my_alarm/zones/control",
     //          "{}",
     //          "Empty JSON object"},

     //          // Complex JSON tests
     //          {"/my_alarm/zones/control",
     //           "{\"zone\":\"Front Door\",\"action\":\"bypass\",\"extra\":\"This should be ignored\"}",
     //           "JSON with extra fields that should be ignored"},
};
