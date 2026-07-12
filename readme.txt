# alarm-core

Alarm core logic with JSON-driven configuration and MQTT command handling.

## JSON handling architecture

The project processes JSON through three layers:

1. **Transport/routing**: `MqttProcessor` (`alarm-core-mqtt.h`)
2. **Parsing/mapping**: `alarmJSON` (`src/alarm-core-JSON.h`)
3. **Domain state/actions**: `Alarm` (`src/alarm-core-class.h`, `src/alarm-core-logic.h`)

The low-level JSON tokenizer/parser is from `esp-json-parser` (`json_parser.h`, `json_parser-code.h`).

---

## Core classes and responsibilities

### `alarmJSON`

Owns the JSON parsing workflow and writes parsed values into `Alarm`.

- **Constructor**
  - `alarmJSON(Alarm& alarm)`

- **Main entry points**
  - `int processConfigJsonPld(const char* jsonBuffer, size_t length)`
    - Parses configuration JSON (`globalOptions`, `zones`, `partitions`, `pgms`, `keyswitches`).
  - `bool processControlJsonPld(const char* jsonBuffer, size_t length, ALARM_DOMAINS_t domain)`
    - Parses control/config payload by explicit domain (`ZONES_CMD`, `PARTITIONS_CMD`, etc.).

- **Domain parsers**
  - `parseGlobalOptionsCfg(...)`
  - `parseZoneCfg(...)`
  - `parseZoneCmd(...)`
  - `parsePartitionCfg(...)`
  - `parsePartitionCmd(...)`
  - `parsePgmCfg(...)` (partial/placeholder)

- **Helpers**
  - `parseJSONval(...)` (typed key extraction)
  - `parse_object(...)` (processor-table driven parsing)
  - `patch_db_item(...)` (patch only fields present in JSON)

It uses processor tables from `src/alarm-core-json-val-parsers.h`:
- `zoneCfgKeyValProcessors`
- `zoneCmdKeyValProcessors`
- `partitionKeyValProcessors`
- `partitionCmdKeyValProcessors`
- `gOptsKeyValProcessors`

---

### `MqttProcessor`

Routes incoming MQTT messages to appropriate JSON processing methods.

- `MqttProcessor(alarmJSON& jsonParser)`
- `main entry point         - bool processIncomingMQTTmsg(const char* topic, const char* payload, size_t length)`
- `process config topic message   - bool processConfigMessage(const char* topic, const char* payload, unsigned int length)`
- `process control topic message  - bool processControlMessage(const char* topic, const char* payload, size_t length)`
- `routing to zones/partitions/etc specific processing - bool processByDomain(ALARM_DOMAINS_t domain, const char* payload, size_t length)`

Routing table (`mqttTopicHandlers[]` in `alarm-core-mqtt.h`):
- `"/alarm/zones/control"` -> `ZONES_CMD`
- `"/alarm/partitions/control"` -> `PARTITIONS_CMD`

**Current design note**
- Routing table is now data-only (`topic`, `domain`, `description`).
- Control routing uses `processByDomain(...)` and `m_jsonParser.processControlJsonPld(...)`.

---

### `Alarm`

Domain model and command executor.

Used by JSON layer for:

- lookup/create entities:
  - `getZoneIndex`, `addZone`
  - `getPartitionIndex`, `addPartition`

- command execution:
  - `modifyZn(...)` (zone command application)
  - `trigerArm(...)` (partition arm/disarm target)

State is persisted in internal DB structs:
- `zonesDB[]`, `zonesRT[]`
- `partitionDB[]`, `partitionRT[]`
- `alarmGlobalOpts`

---

## Interaction wiring

In `alarm-core.cpp`:

- `Alarm my_alarm;`
- `alarmJSON parser(my_alarm);`
- `MqttProcessor myMqttProcessor(parser);`

Dependency chain:

`MqttProcessor` -> `alarmJSON` -> `Alarm`

---

## Call flow summary

1. MQTT message arrives: `(topic, payload, length)`.
2. `processIncomingMQTTmsg(...)` selects path:
   - topic contains `/config` -> `processConfigMessage(...)` -> `processConfigJsonPld(...)`
   - otherwise -> `processControlMessage(...)` -> `processByDomain(...)` -> `processControlJsonPld(...)`
3. `alarmJSON` parses JSON using processor tables and domain-specific handlers.
4. `Alarm` is updated (config patch) or command is triggered (`modifyZn`, `trigerArm`).
