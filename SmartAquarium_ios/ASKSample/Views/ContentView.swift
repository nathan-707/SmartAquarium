/*
See the LICENSE.txt file for this sample’s licensing information.

Abstract:
Main ASK Sample view.
*/

import SwiftUI

struct ContentView: View {
    @EnvironmentObject var aquarium: DiceSessionManager

    var body: some View {

        if aquarium.pickerDismissed, let accessory = aquarium.accessory,
            aquarium.peripheralConnected
        {
            
            
            if aquarium.status == .connected {
                ConnectedMainMenuView()
                    .preferredColorScheme(.dark)

            } else {
                setupWifiView()
                    .preferredColorScheme(.dark)

            }
            

            

        } else if aquarium.pickerDismissed, let accessory = aquarium.accessory,
            !aquarium.peripheralConnected
        {
            SearchingView()
                .preferredColorScheme(.dark)

        } else {
            makeSetupView
            
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

            Text(
                "Hold your iPhone near your Aquarium and make sure it is powered on."
            )
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

}
