import SwiftUI

func mapToUnitRange(_ value: Int) -> Double {
    return Double(value) / 255.0
}

struct ColorTile: Codable, Identifiable, Equatable, Hashable {
    var id: UUID = UUID()
    var red: Double
    var green: Double
    var blue: Double

    func redmapTo255() -> Int {
        return min(max(Int(self.red * 255), 0), 255)
    }

    func greenmapTo255() -> Int {
        return min(max(Int(self.green * 255), 0), 255)
    }

    func bluemapTo255() -> Int {
        return min(max(Int(self.blue * 255), 0), 255)
    }
}

let ColorTiles: [ColorTile] = [
    // off
    ColorTile(red: 0, green: 0.0, blue: 0.0),

    // Red to Yellow (Increase Green)
    ColorTile(red: 1.0, green: 0.0, blue: 0.0),
    ColorTile(red: 1.0, green: 0.2, blue: 0.0),
    ColorTile(red: 1.0, green: 0.4, blue: 0.0),
    ColorTile(red: 1.0, green: 0.6, blue: 0.0),
    ColorTile(red: 1.0, green: 0.8, blue: 0.0),
    ColorTile(red: 1.0, green: 1.0, blue: 0.0),

    // Yellow to Green (Decrease Red)
    ColorTile(red: 0.8, green: 1.0, blue: 0.0),
    ColorTile(red: 0.6, green: 1.0, blue: 0.0),
    ColorTile(red: 0.4, green: 1.0, blue: 0.0),
    ColorTile(red: 0.2, green: 1.0, blue: 0.0),
    ColorTile(red: 0.0, green: 1.0, blue: 0.0),

    // Green to Cyan (Increase Blue)
    ColorTile(red: 0.0, green: 1.0, blue: 0.2),
    ColorTile(red: 0.0, green: 1.0, blue: 0.4),
    ColorTile(red: 0.0, green: 1.0, blue: 0.6),
    ColorTile(red: 0.0, green: 1.0, blue: 0.8),
    ColorTile(red: 0.0, green: 1.0, blue: 1.0),

    // Cyan to Blue (Decrease Green)
    ColorTile(red: 0.0, green: 0.8, blue: 1.0),
    ColorTile(red: 0.0, green: 0.6, blue: 1.0),
    ColorTile(red: 0.0, green: 0.4, blue: 1.0),
    ColorTile(red: 0.0, green: 0.2, blue: 1.0),
    ColorTile(red: 0.0, green: 0.0, blue: 1.0),

    // Blue to Magenta (Increase Red)
    ColorTile(red: 0.2, green: 0.0, blue: 1.0),
    ColorTile(red: 0.4, green: 0.0, blue: 1.0),
    ColorTile(red: 0.6, green: 0.0, blue: 1.0),
    ColorTile(red: 0.8, green: 0.0, blue: 1.0),
    ColorTile(red: 1.0, green: 0.0, blue: 1.0),

    // Magenta to Red (Decrease Blue)
    ColorTile(red: 1.0, green: 0.0, blue: 0.8),
    ColorTile(red: 1.0, green: 0.0, blue: 0.6),
    ColorTile(red: 1.0, green: 0.0, blue: 0.4),
    ColorTile(red: 1.0, green: 0.0, blue: 0.2),
    ColorTile(red: 1.0, green: 0.0, blue: 0.0),

    // white
    ColorTile(red: 1.0, green: 1.0, blue: 1.0),

]
