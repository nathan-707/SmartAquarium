/*
See the LICENSE.txt file for this sample’s licensing information.

Abstract:
Main ASK Sample view.
*/

import SwiftUI

struct ContentView: View {
    
    @EnvironmentObject var aquarium: DiceSessionManager

    var body: some View {
        NavigationStack {
            Group {
                if aquarium.pickerDismissed, let diceColor = aquarium.diceColor {
                    makeRollView(diceColor: diceColor)
                        .navigationTitle(diceColor.displayName)
                } else {
                    makeSetupView
                }
            }
        }
    }

    @ViewBuilder
    private var makeSetupView: some View {
        VStack {
            Spacer()

            Image(systemName: "drop.degreesign")
                .font(.system(size: 150, weight: .light, design: .default))
                .foregroundStyle(.gray)

            Text("No Aquarium")
                .font(Font.title.weight(.bold))
                .padding(.vertical, 12)

            Text("Hold your iPhone near your Aquarium and make sure it is powered on.")
                .font(.subheadline)
                .multilineTextAlignment(.center)

            Spacer()

            Button {
                Task {
                    await aquarium.presentPicker()
                }
            } label: {
                Text("Set Up Aquarium")
                    .frame(maxWidth: .infinity)
                    .font(Font.headline.weight(.semibold))
            }
            .buttonStyle(.bordered)
            .buttonBorderShape(.roundedRectangle)
            .foregroundStyle(.primary)
            .controlSize(.large)
            .padding(.top, 110)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .padding(64)
    }

    @ViewBuilder
    private func makeRollView(diceColor: DiceColor) -> some View {
        VStack {
            Text(aquarium.diceValue.rawValue)
                .font(.system(size: 350).monospaced().bold())
                .foregroundStyle(diceColor.color)
                .contentTransition(.numericText())
                .frame(maxHeight: .infinity)

            Image(diceColor.diceName)
                .resizable()
                .scaledToFit()
                .frame(height: 60)
                .padding(.bottom, 6)

            Button {
                aquarium.peripheralConnected ? aquarium.disconnect() : aquarium.connect()
            } label: {
                Text(aquarium.peripheralConnected ? "Disconnect" : "Connect")
                    .frame(maxWidth: .infinity)
                    .font(Font.headline.weight(.semibold))
            }
            .controlSize(.large)
            .buttonStyle(.borderedProminent)
            .tint(diceColor.color)
            .foregroundStyle(.white)
            .padding(.horizontal, 64)
            .padding(.bottom, 6)

            Button {
                Task {
                    await aquarium.removeDice()
                }
            } label: {
                Text("Remove")
                    .foregroundStyle(.red)
                    .font(Font.headline.weight(.semibold))
            }
            .padding(.bottom, 35)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }
}
