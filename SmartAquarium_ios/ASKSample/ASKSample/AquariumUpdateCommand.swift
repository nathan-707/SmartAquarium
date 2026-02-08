//
//  AquariumUpdateCommand.swift
//  ASKSample
//
//  Created by Nathan Eriksen on 2/8/26.
//  Copyright © 2026 Apple. All rights reserved.
//

import Foundation

enum AquariumUpdateCommand {
    case rgb, bubbler, display
    
    var commandToSerialize: String {
        switch self {
        case .rgb: return "rgb"
        case .bubbler: return "bubbler"
        case .display: return "display"
        }
    }
    
    
}
