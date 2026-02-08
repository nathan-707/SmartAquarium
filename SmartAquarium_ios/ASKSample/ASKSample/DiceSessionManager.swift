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
    @Published var bubbler_isOn: Bool = false
    @Published var heater_isOn: Bool = false
    @Published var display_isOn: Bool = false
    @Published var r_LED: Int = 0
    @Published var g_LED: Int = 255
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

    private static let diceRollCharacteristicUUID = "0xFF3F"



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
    
    func updateAquariumSetting(update: AquariumUpdateCommand){
        
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
            }
        }
    }
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: (any Error)?) {
        
        guard
            error == nil,
            characteristic.uuid == CBUUID(string: Self.diceRollCharacteristicUUID),
            let data = characteristic.value,
            let diceValue = String(data: data, encoding: .utf8)
        else {
            return
        }

        print("New dice value received: \(diceValue)")

        DispatchQueue.main.async {
            withAnimation {
                self.diceValue = DiceValue(rawValue: diceValue) ?? .one
            }
        }
    }
}
