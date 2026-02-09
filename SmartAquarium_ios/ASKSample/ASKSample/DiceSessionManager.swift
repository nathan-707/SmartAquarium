/*
See the LICENSE.txt file for this sample’s licensing information.

Abstract:
Manages the ASAccessorySession and connections to dice accessories.
*/

import Foundation
import AccessorySetupKit
import CoreBluetooth
import SwiftUI

class DiceSessionManager: NSObject, ObservableObject {
    
// mark: settings
    
    @Published var settings = ""
    @Published var bubbler_isOn: Bool = false
    @Published var heater_isOn: Bool = false
    @Published var display_isOn: Bool = false
    @Published var r_LED: Int = 0
    @Published var g_LED: Int = 0
    @Published var b_LED: Int = 0

    // mark: readings
    @Published var tds_level: Float = 0.0
    @Published var water_temp: Float = 0.0
    @Published var daysSinceFed: Int = 0
    
    

    
    @Published var diceColor: DiceColor?
    @Published var diceValue = DiceValue.one
    @Published var peripheralConnected = false
    @Published var pickerDismissed = true

    private var currentDice: ASAccessory?
    private var session = ASAccessorySession()
    private var manager: CBCentralManager?
    private var peripheral: CBPeripheral?
    private var rollResultCharacteristic: CBCharacteristic?
    private var iosCommandCharacteristic: CBCharacteristic?

    private static let diceRollCharacteristicUUID = "0xFF3F"
    private static let CHARACTERISTIC_UUID_IOS = "0xBB3B"



    private static let blueDice: ASPickerDisplayItem = {
        let descriptor = ASDiscoveryDescriptor()
        descriptor.bluetoothServiceUUID = DiceColor.blue.serviceUUID

        return ASPickerDisplayItem(
            name: DiceColor.blue.displayName,
            productImage: UIImage(named: DiceColor.blue.diceName)!,
            descriptor: descriptor
        )
    }()

    override init() {
        super.init()
        self.session.activate(on: DispatchQueue.main, eventHandler: handleSessionEvent(event:))
    }

    // MARK: - DiceSessionManager actions

    func presentPicker() async {
        do {
            try await session.showPicker(for: [ Self.blueDice])
        } catch let error {
            print("Failed to show picker due to: \(error.localizedDescription)")
        }
    }

    func removeDice() async {
        guard let currentDice else { return }

        if peripheralConnected {
            disconnect()
        }

        do {
            try await session.removeAccessory(currentDice)
            self.diceColor = nil
            self.currentDice = nil
            self.manager = nil
        } catch let error {
            print("Failed to remove accessory due to: \(error.localizedDescription)")
            return
        }
    }

    func connect() {
        guard
            let manager, manager.state == .poweredOn,
            let peripheral
        else {
            return
        }

        manager.connect(peripheral)
    }

    func disconnect() {
        guard let peripheral, let manager else { return }
        manager.cancelPeripheralConnection(peripheral)
    }

    // MARK: - ASAccessorySession functions

    private func saveDice(dice: ASAccessory) {
        currentDice = dice

        if manager == nil {
            manager = CBCentralManager(delegate: self, queue: nil)
        }

            diceColor = .blue
        
    }

    private func handleSessionEvent(event: ASAccessoryEvent) {
        switch event.eventType {
        case .accessoryAdded, .accessoryChanged:
            guard let dice = event.accessory else { return }
            saveDice(dice: dice)
        case .activated:
            guard let dice = session.accessories.first else { return }
            saveDice(dice: dice)
            connect()

        case .accessoryRemoved:
            self.diceColor = nil
            self.currentDice = nil
            self.manager = nil
        case .pickerDidPresent:
            pickerDismissed = false
        case .pickerDidDismiss:
            pickerDismissed = true
        default:
            print("Received event type \(event.eventType)")
        }
    }
    
    func pingAquarium(){
        print("pinging aquarium")
    }
    
    private struct IOSCommandPayload: Codable {
        let command: String
    }
    
    private var red = 10
    private var green = 20
    private var blue = 30


    func updateAquariumSetting(update: String){
        print("Sending command: \(update)")
        if let peripheral = peripheral, let iosCommandCharacteristic = iosCommandCharacteristic {
            red += 10
            green += 10
            blue += 10
            
            
            struct Command: Codable {
                var command: String
                var value: String
                var value2: String
                var value3: String
            }
            let toESP = Command(
                command: "RGB",
                value: String(red),
                value2: String(green),
                value3: String(blue)
            )
            peripheral.writeValue(
                encodeTOJSON(any: toESP),
                for: iosCommandCharacteristic,
                type: .withResponse
            )
            
            
        }
    }
}


// MARK: - CBCentralManagerDelegate

extension DiceSessionManager: CBCentralManagerDelegate {
    
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        print("Central manager state: \(central.state)")
        switch central.state {
        case .poweredOn:
            if let peripheralUUID = currentDice?.bluetoothIdentifier {
                peripheral = central.retrievePeripherals(withIdentifiers: [peripheralUUID]).first
                peripheral?.delegate = self
            }
        default:
            peripheral = nil
        }
    }

    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("Connected to peripheral: \(peripheral)")
        guard let diceColor else { return }
        peripheral.delegate = self
        
        peripheral.discoverServices([diceColor.serviceUUID])
        
        peripheralConnected = true
    }

    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: (any Error)?) {
        print("Disconnected from peripheral: \(peripheral)")
        peripheralConnected = false
    }

    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: (any Error)?) {
        print("Failed to connect to peripheral: \(peripheral), error: \(error.debugDescription)")
    }
}

// MARK: - CBPeripheralDelegate

extension DiceSessionManager: CBPeripheralDelegate {
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: (any Error)?) {
        
        print("service found!")
        
        guard
            error == nil,
            let services = peripheral.services
        else {
            return
        }
        
        for service in services {
            peripheral.discoverCharacteristics(nil, for: service)
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: (any Error)?) {
        // 1. Check for errors
        if let error = error {
            print("Error discovering characteristics: \(error.localizedDescription)")
            return
        }
        
        guard let characteristics = service.characteristics else { return }
        
        // 2. Loop through EVERYTHING found (Don't filter yet!)
        for characteristic in characteristics {
            print("Found Characteristic: \(characteristic.uuid.uuidString)") // DEBUG PRINT
            
            // 3. Check for match (Compare strings to be safe)
            // This handles "FF3F" vs "0xFF3F" vs "ff3f" automatically
            if characteristic.uuid.uuidString.caseInsensitiveCompare("FF3F") == .orderedSame ||
                characteristic.uuid.uuidString == "0xFF3F" {
                
                print("MATCH FOUND! Subscribing to notifications...")
                
                rollResultCharacteristic = characteristic
                
                // 4. CRITICAL: This tells the ESP32 "Start sending me data!"
                peripheral.setNotifyValue(true, for: characteristic)
                
                // 5. Read the initial value
                peripheral.readValue(for: characteristic)
            } else if characteristic.uuid.uuidString.caseInsensitiveCompare("BB3B") == .orderedSame ||
                            characteristic.uuid.uuidString == "0xBB3B" {
                print("ios characteristic found!")
                iosCommandCharacteristic = characteristic
            }
        }
    }
    
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: (any Error)?) {
        // 1. Basic Safety Check
        guard error == nil, let data = characteristic.value else { return }
        
        // 2. Route based on which characteristic sent data
        switch characteristic.uuid {
            
        case CBUUID(string: Self.diceRollCharacteristicUUID):
            handleSettingsUpdate(from: data)
            
        case CBUUID(string: Self.CHARACTERISTIC_UUID_IOS):
            print("IOS")
            break
            
        default:
            print("Received update for unknown characteristic: \(characteristic.uuid)")
        }
    }
    
    private func handleSettingsUpdate(from data: Data) {
        let decoder = JSONDecoder()
        
        do {
            // This replaces your 'settings = String(diceValue)' line
            let decodedSettings = try decoder.decode(ESPSettings.self, from: data)
            
            // Now you have actual variables!
            print("Bubbler is now: \(decodedSettings.bubbler)")
            print("Color is: R\(decodedSettings.r) G\(decodedSettings.g) B\(decodedSettings.b)")
            
            // Update your UI or State here
            r_LED = decodedSettings.r
            g_LED = decodedSettings.g
            b_LED = decodedSettings.b
            
            display_isOn = decodedSettings.display
            bubbler_isOn = decodedSettings.bubbler
            
        } catch {
            print("Failed to decode JSON: \(error)")
        }
    }
    
}

struct ESPSettings: Codable {
    let bubbler: Bool
    let display: Bool
    let r: Int
    let g: Int
    let b: Int
}

