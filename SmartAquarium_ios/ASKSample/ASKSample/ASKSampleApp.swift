/*
See the LICENSE.txt file for this sample’s licensing information.

Abstract:
Launches the main app view for ASKSample.
*/

import SwiftUI

@main
struct ASKSampleApp: App {
    
    @State var aquariumSessionManager = DiceSessionManager()

    
    

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environmentObject(aquariumSessionManager)
        }
    }
}
