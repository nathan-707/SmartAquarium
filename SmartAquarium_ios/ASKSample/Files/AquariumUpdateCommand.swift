//
//  AquariumUpdateCommand.swift
//  ASKSample
//
//  Created by Nathan Eriksen on 2/8/26.
//  Copyright © 2026 Apple. All rights reserved.
//

import Foundation

func updateRGBCommand(r: Int, b: Int, g: Int) -> String {
    return "command: rgb, value1: \(r), value2: \(g), value3: \(b)"
}

func encodeTOJSON(any: Codable) -> Data {
    do {
        let encoder = JSONEncoder()
        let jsonData = try encoder.encode(any)

        if let jsonString = String(data: jsonData, encoding: .utf8) {
            print("Writing with response: \(jsonString)")
        } else {
            print("Writing with response: command updated")
        }
        return jsonData
    } catch {
        print("Failed to encode command: \(error)")
    }
    return Data()
}

enum LightCycle: Int, Codable {
    case standard,
         noSchedule
}




