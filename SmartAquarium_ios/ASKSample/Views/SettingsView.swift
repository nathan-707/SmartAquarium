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

    var body: some View {
        
        Form {
            Toggle("Bubbler", isOn: $aquarium.bubbler_isOn)
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
        
        .onChange(of: aquarium.bubbler_isOn, { oldValue, newValue in
            aquarium.updateAquariumSetting(command: "bubbler", value: aquarium.bubbler_isOn ? "true" : "false")
        })
        .task {
            redValue = aquarium.r_LED
            greenValue = aquarium.g_LED
            blueValue = aquarium.b_LED
        }

    }
}

#Preview {
    SettingsView()
}
