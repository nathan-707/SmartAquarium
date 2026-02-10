//
//  SearchingView.swift
//  ASKSample
//
//  Created by Nathan Eriksen on 2/10/26.
//  Copyright © 2026 Apple. All rights reserved.
//

import SwiftUI

struct SearchingView: View {
    @EnvironmentObject var aquarium: DiceSessionManager

    var body: some View {
        VStack{
            Text("Searching for Aquarium")
            ProgressView()
        }
            .foregroundStyle(.secondary)
            .task {
                while !Task.isCancelled {
                    aquarium.connect()
                    try? await Task.sleep(for: .seconds(3))
                }
            }
            .onAppear(){
                aquarium.connect()
            }
    }
}

#Preview {
    SearchingView()
}
