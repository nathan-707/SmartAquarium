//
//  setupWifiView.swift
//  ASKSample
//
//  Created by Nathan Eriksen on 2/15/26.
//  Copyright © 2026 Apple. All rights reserved.
//

import SwiftUI

struct setupWifiView: View {
    @EnvironmentObject var aquarium: DiceSessionManager
    

    var body: some View {

        VStack {

       
                Text("Enter WiFi Network for Aquarium")
                .font(.title)
                .bold()
            
            Spacer()
            

            VStack(alignment: .leading, spacing: 8) {
                Text("SSID")
                    .font(.subheadline)
                TextField("Enter network name", text: $aquarium.ssid)
                    .textFieldStyle(.roundedBorder)
                    .textInputAutocapitalization(.never)
                    .autocorrectionDisabled(true)
                    .onSubmit {
                        print("Password submitted: \(aquarium.password)")
                        aquarium.updateAquariumSetting(
                            command: "password",
                            value: aquarium.password
                        )
                        print("SSID submitted: \(aquarium.ssid)")
                        aquarium.updateAquariumSetting(
                            command: "ssid",
                            value: aquarium.ssid
                        )
                    }
            }

            VStack(alignment: .leading, spacing: 8) {
                Text("Password")
                    .font(.subheadline)
                SecureField("Enter password", text: $aquarium.password)
                    .textFieldStyle(.roundedBorder)
                    .onSubmit {
                        print("Password submitted: \(aquarium.password)")
                        aquarium.updateAquariumSetting(
                            command: "password",
                            value: aquarium.password
                        )
                        print("SSID submitted: \(aquarium.ssid)")
                        aquarium.updateAquariumSetting(
                            command: "ssid",
                            value: aquarium.ssid
                        )
                    }
            }

        }
        .padding()

    }
}

#Preview {
    setupWifiView()
}
