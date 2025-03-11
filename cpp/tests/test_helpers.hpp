#pragma once
#include <memory>
#include "logger.hpp"

namespace TestHelpers {
    // Class to temporarily disable waiting in tests
    class ScopedWaitDisabler {
    private:
        bool originalValue;
        
    public:
        ScopedWaitDisabler() {
            // Get the current configuration
            auto& logger = Logger2048::Logger::getInstance();
            const auto& currentConfig = logger.getConfig();
            
            // Store the original waitEnabled value
            originalValue = currentConfig.waitEnabled;
            
            // Create a new config with waitEnabled = false
            if (originalValue) {
                Logger2048::LoggerConfig newConfig = currentConfig;
                newConfig.waitEnabled = false;
                logger.configure(newConfig);
            }
        }
        
        ~ScopedWaitDisabler() {
            // Restore original value only if it was true
            if (originalValue) {
                auto& logger = Logger2048::Logger::getInstance();
                const auto& currentConfig = logger.getConfig();
                Logger2048::LoggerConfig newConfig = currentConfig;
                newConfig.waitEnabled = originalValue;
                logger.configure(newConfig);
            }
        }
    };
} 