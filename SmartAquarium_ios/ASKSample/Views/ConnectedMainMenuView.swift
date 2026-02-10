//
//  ConnectedMainMenuView.swift
//  ASKSample
//
//  Created by Nathan Eriksen on 2/9/26.
//  Copyright © 2026 Apple. All rights reserved.
//

import SwiftUI

struct ConnectedMainMenuView: View {
    
    @EnvironmentObject var aquarium: DiceSessionManager
    @State var bubblerToggle: Bool = false

    @State private var presentSettings = false

    private var primaryColor: Color {
        let r = Double(max(0, min(255, aquarium.r_LED))) / 255.0
        let g = Double(max(0, min(255, aquarium.g_LED))) / 255.0
        let b = Double(max(0, min(255, aquarium.b_LED))) / 255.0
        return Color(red: r, green: g, blue: b)
    }



    var body: some View {
     
        ZStack {
            LinearGradient(colors: [primaryColor, Color(.systemBackground)], startPoint: .bottomTrailing, endPoint: .topLeading)
                .ignoresSafeArea()

            VStack {
                HStack {
                    Text("Aquarium")
                        .font(.title2).fontWeight(.semibold)
                        .foregroundStyle(.primary)
                    Spacer()
                    Button("", systemImage: "gear") {
                        presentSettings = true
                    }
                    .tint(.green)
                }
                .padding(.horizontal)
                .padding(.top)
                
                
                VStack(spacing: 12) {
                    ReadingCard(title: "Water Temp", value: String(aquarium.water_temp), unit: "°C", symbol: "thermometer.medium")
                    ReadingCard(title: "TDS", value: String(aquarium.tds_level), unit: "ppm", symbol: "bubbles.and.sparkles.fill")
                    ReadingCard(title: "Days Since Fed", value: String(aquarium.daysSinceFed), unit: "days", symbol: "fish.fill")
                }
                .padding(.horizontal)
                .padding(.top, 8)
              
               
//                
//                Button{
//                    aquarium.pingAquarium()
//                } label: {
//                    Text("PING")
//                }.padding()
//
//                Button {
//                    Task {
//                        await aquarium.removeDice()
//                    }
//                } label: {
//                    Text("Remove")
//                        .foregroundStyle(.red)
//                        .font(Font.headline.weight(.semibold))
//                }
//                .padding(.bottom, 35)
                
                
//                Button {
//                    aquarium.peripheralConnected ? aquarium.disconnect() : aquarium.connect()
//                } label: {
//                    Text(aquarium.peripheralConnected ? "Disconnect" : "Connect")
//                        .frame(maxWidth: .infinity)
//                        .font(Font.headline.weight(.semibold))
//                }
//                .controlSize(.large)
//                .buttonStyle(.borderedProminent)
//                .foregroundStyle(.white)
//                .padding(.horizontal, 64)
//                .padding(.bottom, 6)
                
                Spacer()
                
            }
        }
        .sheet(isPresented: $presentSettings) {
            SettingsView()
        }
    }
    
    private struct ReadingCard: View {
        let title: String
        let value: String
        let unit: String
        let symbol: String

        var body: some View {
            ZStack {
                RoundedRectangle(cornerRadius: 16, style: .continuous)
                    .fill(Color.green.opacity(0.15))
                    .overlay(
                        RoundedRectangle(cornerRadius: 16, style: .continuous)
                            .stroke(Color.green.opacity(0.35), lineWidth: 1)
                    )
                HStack {
                    VStack(alignment: .leading, spacing: 4) {
                        Text(title)
                            .font(.caption)
                            .foregroundStyle(.secondary)
                        HStack(alignment: .firstTextBaseline, spacing: 6) {
                            Text(value)
                                .font(.system(size: 28, weight: .semibold, design: .rounded))
                                .foregroundStyle(.green)
                            Text(unit)
                                .font(.subheadline)
                                .foregroundStyle(.secondary)
                        }
                    }
                    Spacer()
                    
                    Image(systemName: symbol)
                        .foregroundStyle(.green)
                        .imageScale(.large)
                        .opacity(0.8)
                }
                .padding(16)
            }
            .frame(maxWidth: .infinity)
            .frame(height: 80)
        }
    }
}

#Preview {
    ConnectedMainMenuView()
}
