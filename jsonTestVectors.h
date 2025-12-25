#pragma once
// This array contains the test vectors for the JSON MQTT commands
JsonMQTTTestVector jsonTestVectors[] = {
    //// Zone control tests
    //{"/alarm/config",
    // jsonBuffer,
    // "Test config JSON"},

     // Zone control tests
     {"/alarm/zones/control",
      "{\"zName\":\"Front Door\",\"action\":\"bypass\"}",
      "Bypass Front Door zone"},

     //{"/alarm/zones/control",
     // "{\"zone\":\"Back Door\",\"action\":\"clear_bypass\"}",
     // "Clear bypass on Back Door zone"},

     //{"/alarm/zones/control",
     // "{\"zone\":\"Motion Sensor\",\"action\":\"tamper\"}",
     // "Trigger tamper on Motion Sensor zone"},

     //{"/alarm/zones/control",
     // "{\"zone\":\"Window Sensor\",\"action\":\"open\"}",
     // "Open Window Sensor zone"},

     //{"/alarm/zones/control",
     // "{\"zone\":\"Smoke Detector\",\"action\":\"close\"}",
     // "Close Smoke Detector zone"},

     //{"/alarm/zones/control",
     // "{\"zone\":\"PIR Living Room\",\"action\":\"anti-mask\"}",
     // "Trigger anti-mask on PIR Living Room zone"},

     // // Invalid zone tests
     // {"/alarm/zones/control",
     //  "{\"zone\":\"InvalidZone\",\"action\":\"bypass\"}",
     //  "Bypass an invalid zone"},

     // {"/alarm/zones/control",
     //  "{\"zone\":\"Front Door\",\"action\":\"invalid-action\"}",
     //  "Invalid action for a valid zone"},

     // {"/alarm/zones/control",
     //  "{\"invalid-key\":\"Front Door\",\"action\":\"bypass\"}",
     //  "Invalid JSON key for zone control"},

     // {"/alarm/zones/control",
     //  "{\"zone\":\"Front Door\"}",
     //  "Missing action key in JSON"},

     // {"/alarm/zones/control",
     //  "{\"action\":\"bypass\"}",
     //  "Missing zone key in JSON"},

     //  // Partition control tests
     //  {"/alarm/partitions/control",
     //   "{\"partition\":\"Main Floor\",\"action\":\"disarm\"}",
     //   "Disarm Main Floor partition"},

     //  {"/alarm/partitions/control",
     //   "{\"partition\":\"Upper Floor\",\"action\":\"arm\"}",
     //   "Arm Upper Floor partition"},

     //  {"/alarm/partitions/control",
     //   "{\"partition\":\"Basement\",\"action\":\"arm_force\"}",
     //   "Force arm Basement partition"},

     //  {"/alarm/partitions/control",
     //   "{\"partition\":\"Garage\",\"action\":\"arm_stay\"}",
     //   "Stay arm Garage partition"},

     //  {"/alarm/partitions/control",
     //   "{\"partition\":\"Office\",\"action\":\"arm_instant\"}",
     //   "Instant arm Office partition"},

     //   // Invalid partition tests
     //   {"/alarm/partitions/control",
     //    "{\"partition\":\"InvalidPartition\",\"action\":\"arm\"}",
     //    "Arm an invalid partition"},

     //   {"/alarm/partitions/control",
     //    "{\"partition\":\"Main Floor\",\"action\":\"invalid-action\"}",
     //    "Invalid action for a valid partition"},

     //    // PGM control tests
     //    {"/alarm/pgms/control",
     //     "{\"pgm\":\"Siren\",\"action\":\"on\"}",
     //     "Turn on Siren PGM"},

     //    {"/alarm/pgms/control",
     //     "{\"pgm\":\"Garage Door\",\"action\":\"off\"}",
     //     "Turn off Garage Door PGM"},

     //    {"/alarm/pgms/control",
     //     "{\"pgm\":\"Outdoor Lights\",\"action\":\"pulse\"}",
     //     "Pulse Outdoor Lights PGM"},

     //     // Invalid PGM tests
     //     {"/alarm/pgms/control",
     //      "{\"pgm\":\"InvalidPGM\",\"action\":\"on\"}",
     //      "Turn on an invalid PGM"},

     //     {"/alarm/pgms/control",
     //      "{\"pgm\":\"Siren\",\"action\":\"invalid-action\"}",
     //      "Invalid action for a valid PGM"},

     //      // Global options tests
     //      {"/alarm/global/options",
     //       "{\"option\":\"MaxSlaves\",\"value\":\"5\"}",
     //       "Set MaxSlaves option to 5"},

     //      {"/alarm/global/options",
     //       "{\"option\":\"RestrSprvsL\",\"value\":\"true\"}",
     //       "Enable restriction on supervision loss"},

     //      {"/alarm/global/options",
     //       "{\"option\":\"RestrTamper\",\"value\":\"false\"}",
     //       "Disable restriction on tamper"},

     //      {"/alarm/global/options",
     //       "{\"option\":\"RestrACfail\",\"value\":\"true\"}",
     //       "Enable restriction on AC failure"},

     //       // Invalid global options tests
     //       {"/alarm/global/options",
     //        "{\"option\":\"InvalidOption\",\"value\":\"true\"}",
     //        "Set an invalid global option"},

     //       {"/alarm/global/options",
     //        "{\"option\":\"MaxSlaves\",\"value\":\"invalid\"}",
     //        "Set MaxSlaves to an invalid value"},

     //        // Malformed JSON tests
     //        {"/alarm/zones/control",
     //         "{\"zone\":\"Front Door\",\"action\":\"bypass\"",
     //         "Malformed JSON - missing closing brace"},

     //        {"/alarm/partitions/control",
     //         "This is not JSON",
     //         "Invalid JSON format"},

     //        {"/alarm/pgms/control",
     //         "",
     //         "Empty payload"},

     //         // Empty JSON tests
     //         {"/alarm/zones/control",
     //          "{}",
     //          "Empty JSON object"},

     //          // Complex JSON tests
     //          {"/alarm/zones/control",
     //           "{\"zone\":\"Front Door\",\"action\":\"bypass\",\"extra\":\"This should be ignored\"}",
     //           "JSON with extra fields that should be ignored"},
};
