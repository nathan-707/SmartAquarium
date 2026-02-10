/*
See the LICENSE.txt file for this sample’s licensing information.

Abstract:
Defines dice color options.
*/

import CoreBluetooth
import SwiftUI

enum SmartAquarium: String {
    case blue

    var color: Color {
        switch self {
            case .blue: .cyan
        }
    }

    var displayName: String {
        "Smart Aquarium"
    }

    var diceName: String {
        "\(self.rawValue)"
    }

    var serviceUUID: CBUUID {
        switch self {
            case .blue: CBUUID(string: "E56A082E-C49B-47CA-A2AB-389127B8ABE7")
        }
    }
}
