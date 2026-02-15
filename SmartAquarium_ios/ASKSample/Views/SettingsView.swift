//
//  SettingsView.swift
//  ASKSample
//
//  Created by Nathan Eriksen on 2/10/26.
//  Copyright © 2026 Apple. All rights reserved.
//

import SwiftUI

struct SettingsView: View {
    @State private var redValue: Int = 0
    @State private var greenValue: Int = 0
    @State private var blueValue: Int = 0
    @EnvironmentObject var aquarium: DiceSessionManager
    @State var selectedTile = ColorTile(red: 255, green: 0, blue: 0)
    @State var togg: Bool = false
    @State private var onTimeDate: Date = Date()
    @State private var offTimeDate: Date = Date()
    @State var showChangeWifi = false

    var body: some View {
        


        Form {
        
            Toggle("Bubbler", isOn: $aquarium.bubbler_isOn)
            
            Stepper(value: $aquarium.temp_Warning_thres, in: 0...120, step: 1) {
                HStack {
                    Text("Temp Warning Threshold")
                    Spacer()
                    Text(
                        String(
                            format: "%.2f",
                            Double(aquarium.temp_Warning_thres)
                        )
                    )
                }
            }
            .onChange(of: aquarium.temp_Warning_thres) { oldValue, newValue in
                aquarium.updateAquariumSetting(
                    command: "temp_Warning_thres",
                    value: String(newValue)
                )
            }
            
            Stepper(value: $aquarium.targetTemp, in: 0...120, step: 1) {
                HStack {
                    Text("Target Temp")
                    Spacer()
                    Text(String(format: "%.2f", Double(aquarium.targetTemp)))
                }
            }
            .onChange(of: aquarium.targetTemp) { oldValue, newValue in
                aquarium.updateAquariumSetting(
                    command: "targetTemp",
                    value: String(newValue)
                )
            }
            
            Stepper(value: $aquarium.tds_Warning_thres, in: 0...2000, step: 10)
            {
                HStack {
                    Text("TDS Warning Threshold")
                    Spacer()
                    Text(
                        String(
                            format: "%.2f",
                            Double(aquarium.tds_Warning_thres)
                        )
                    )
                }
            }
            .onChange(of: aquarium.tds_Warning_thres) { oldValue, newValue in
                aquarium.updateAquariumSetting(
                    command: "tds_Warning_thres",
                    value: String(newValue)
                )
            }
            
            Stepper(value: $aquarium.daysFed_Warning_thres, in: 0...30, step: 1)
            {
                HStack {
                    Text("Days Fed Warning Threshold")
                    Spacer()
                    Text(
                        String(
                            format: "%.2f",
                            Double(aquarium.daysFed_Warning_thres)
                        )
                    )
                }
            }
            .onChange(of: aquarium.daysFed_Warning_thres) {
                oldValue,
                newValue in
                aquarium.updateAquariumSetting(
                    command: "daysFed_Warning_thres",
                    value: String(newValue)
                )
            }
            
            Toggle("Lamp", isOn: $aquarium.lamp_isOn)
                .onChange(of: aquarium.lamp_isOn) { oldValue, newValue in
                    aquarium.updateAquariumSetting(
                        command: "lamp",
                        value: newValue ? "true" : "false"
                    )
                }
            
            Section("On Time") {
                DatePicker(
                    "Time",
                    selection: $onTimeDate,
                    displayedComponents: .hourAndMinute
                )
                .datePickerStyle(.wheel)
                .onAppear {
                    var comps = DateComponents()
                    comps.hour = aquarium.onTimeHr
                    comps.minute = aquarium.onTimeMin
                    onTimeDate = Calendar.current.date(from: comps) ?? Date()
                }
                .onChange(of: onTimeDate) { _, newValue in
                    let comps = Calendar.current.dateComponents(
                        [.hour, .minute],
                        from: newValue
                    )
                    let hr = comps.hour ?? 0
                    let min = comps.minute ?? 0
                    if aquarium.onTimeHr != hr {
                        aquarium.onTimeHr = hr
                        aquarium.updateAquariumSetting(
                            command: "onTimeHr",
                            value: String(hr)
                        )
                    }
                    if aquarium.onTimeMin != min {
                        aquarium.onTimeMin = min
                        aquarium.updateAquariumSetting(
                            command: "onTimeMin",
                            value: String(min)
                        )
                    }
                }
                .onChange(of: aquarium.onTimeHr) { _, _ in
                    var comps = DateComponents()
                    comps.hour = aquarium.onTimeHr
                    comps.minute = aquarium.onTimeMin
                    onTimeDate =
                    Calendar.current.date(from: comps) ?? onTimeDate
                }
                .onChange(of: aquarium.onTimeMin) { _, _ in
                    var comps = DateComponents()
                    comps.hour = aquarium.onTimeHr
                    comps.minute = aquarium.onTimeMin
                    onTimeDate =
                    Calendar.current.date(from: comps) ?? onTimeDate
                }
            }
            
            Section("Off Time") {
                DatePicker(
                    "Time",
                    selection: $offTimeDate,
                    displayedComponents: .hourAndMinute
                )
                .datePickerStyle(.wheel)
                .onAppear {
                    var comps = DateComponents()
                    comps.hour = aquarium.offTimeHr
                    comps.minute = aquarium.offTimeMin
                    offTimeDate = Calendar.current.date(from: comps) ?? Date()
                }
                .onChange(of: offTimeDate) { _, newValue in
                    let comps = Calendar.current.dateComponents(
                        [.hour, .minute],
                        from: newValue
                    )
                    let hr = comps.hour ?? 0
                    let min = comps.minute ?? 0
                    if aquarium.offTimeHr != hr {
                        aquarium.offTimeHr = hr
                        aquarium.updateAquariumSetting(
                            command: "offTimeHr",
                            value: String(hr)
                        )
                    }
                    if aquarium.offTimeMin != min {
                        aquarium.offTimeMin = min
                        aquarium.updateAquariumSetting(
                            command: "offTimeMin",
                            value: String(min)
                        )
                    }
                }
                .onChange(of: aquarium.offTimeHr) { _, _ in
                    var comps = DateComponents()
                    comps.hour = aquarium.offTimeHr
                    comps.minute = aquarium.offTimeMin
                    offTimeDate =
                    Calendar.current.date(from: comps) ?? offTimeDate
                }
                .onChange(of: aquarium.offTimeMin) { _, _ in
                    var comps = DateComponents()
                    comps.hour = aquarium.offTimeHr
                    comps.minute = aquarium.offTimeMin
                    offTimeDate =
                    Calendar.current.date(from: comps) ?? offTimeDate
                }
            }
            
            VStack(alignment: .leading, spacing: 12) {
                
                Picker("colorpicker", selection: $selectedTile) {
                    ForEach(ColorTiles) { tile in
                        let color = Color(
                            red: Double(tile.red),
                            green: Double(tile.green),
                            blue: Double(tile.blue)
                        )
                        Rectangle().frame(width: 50, height: 50)
                            .foregroundColor(color)
                            .padding(5)
                            .tag(tile)
                    }
                }
                .pickerStyle(.wheel)
                
            }
            
            
            if showChangeWifi == false {
                Button {
                    showChangeWifi = true
                } label: {
                    Text("Change Wifi")
                }
                
            }
            
            if showChangeWifi {
                setupWifiView()
            }
            
            
            
            
            
            
        
         

        }

        .onChange(of: selectedTile) { oldValue, newValue in
            redValue = selectedTile.redmapTo255()
            greenValue = selectedTile.greenmapTo255()
            blueValue = selectedTile.bluemapTo255()
            aquarium.updateAquariumSetting(
                command: "RGB",
                value: String(redValue),
                value2: String(greenValue),
                value3: String(blueValue)
            )
            aquarium.pingAquarium()
        }

        .onChange(
            of: aquarium.bubbler_isOn,
            { oldValue, newValue in
                aquarium.updateAquariumSetting(
                    command: "bubbler",
                    value: aquarium.bubbler_isOn ? "true" : "false"
                )
            }
        )
        .task {
            redValue = aquarium.r_LED
            greenValue = aquarium.g_LED
            blueValue = aquarium.b_LED
        }

    }

    private static func format12Hour(hour: Int, minute: Int) -> String {
        let h = hour % 24
        let m = max(0, min(59, minute))
        let ampm = h < 12 ? "AM" : "PM"
        let hour12 = h % 12 == 0 ? 12 : h % 12
        return String(format: "%d:%02d %@", hour12, m, ampm)
    }
}

#Preview {
    SettingsView()
}
