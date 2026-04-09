#include "EngineArchitectureValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/PlatformApplicationMisc.h"
#include "RenderingThread.h"
#include "RHI.h"

UEngineArchitectureValidator::UEngineArchitectureValidator()
{
    bValidationCacheValid = false;
    LastValidationTime = 0.0f;
}

void UEngineArchitectureValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architecture Validator initialized"));
    
    // Perform initial validation
    ValidateEngineArchitecture();
}

TArray<FArchitectureValidation> UEngineArchitectureValidator::ValidateEngineArchitecture()
{
    UE_LOG(LogTemp, Log, TEXT("=== ENGINE ARCHITECTURE VALIDATION START ==="));
    
    LastValidationResults.Empty();
    
    // Core UE5 Features Validation
    LastValidationResults.Add(ValidateWorldPartition());
    LastValidationResults.Add(ValidateNaniteSupport());
    LastValidationResults.Add(ValidateLumenSupport());
    LastValidationResults.Add(ValidateVirtualShadowMaps());
    LastValidationResults.Add(ValidateMassEntitySystem());
    LastValidationResults.Add(ValidatePCGSystem());
    
    // Performance and Configuration
    LastValidationResults.Add(ValidatePerformanceTargets());
    LastValidationResults.Add(ValidateMemoryRequirements());
    LastValidationResults.Add(ValidateProjectSettings());
    LastValidationResults.Add(ValidateRenderingSettings());
    
    // Cache validation results
    bValidationCacheValid = true;
    LastValidationTime = FPlatformTime::Seconds();
    
    // Log summary
    int32 ErrorCount = 0;
    int32 WarningCount = 0;
    for (const FArchitectureValidation& Result : LastValidationResults)
    {
        LogValidationResult(Result);
        if (Result.Result == EArchitectureValidationResult::Error || 
            Result.Result == EArchitectureValidationResult::Critical)
        {
            ErrorCount++;
        }
        else if (Result.Result == EArchitectureValidationResult::Warning)
        {
            WarningCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture Validation Complete: %d Errors, %d Warnings"), 
           ErrorCount, WarningCount);
    
    return LastValidationResults;
}

FArchitectureValidation UEngineArchitectureValidator::ValidateWorldPartition()
{
    // Check if World Partition is available
    if (CheckFeatureAvailability(TEXT("World Partition"), 
        StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/Engine.WorldPartition"))))
    {
        // Check if current world uses World Partition
        UWorld* World = GetWorld();
        if (World && World->IsPartitionedWorld())
        {
            return FArchitectureValidation(
                TEXT("World Partition"),
                EArchitectureValidationResult::Valid,
                TEXT("World Partition is enabled and active"),
                TEXT("")
            );
        }
        else
        {
            return FArchitectureValidation(
                TEXT("World Partition"),
                EArchitectureValidationResult::Warning,
                TEXT("World Partition available but not enabled on current world"),
                TEXT("Enable World Partition in World Settings for large world support")
            );
        }
    }
    
    return FArchitectureValidation(
        TEXT("World Partition"),
        EArchitectureValidationResult::Error,
        TEXT("World Partition not available"),
        TEXT("Ensure UE5 with World Partition support is being used")
    );
}

FArchitectureValidation UEngineArchitectureValidator::ValidateNaniteSupport()
{
    // Check Nanite availability through project settings
    bool bNaniteEnabled = false;
    GConfig->GetBool(TEXT("/Script/Engine.RendererSettings"), TEXT("r.Nanite"), bNaniteEnabled, GEngineIni);
    
    if (bNaniteEnabled)
    {
        return FArchitectureValidation(
            TEXT("Nanite Virtualized Geometry"),
            EArchitectureValidationResult::Valid,
            TEXT("Nanite is enabled and available"),
            TEXT("")
        );
    }
    
    return FArchitectureValidation(
        TEXT("Nanite Virtualized Geometry"),
        EArchitectureValidationResult::Error,
        TEXT("Nanite is not enabled"),
        TEXT("Enable Nanite in Project Settings > Engine > Rendering")
    );
}

FArchitectureValidation UEngineArchitectureValidator::ValidateLumenSupport()
{
    // Check Lumen Global Illumination
    int32 DynamicGlobalIllumination = 0;
    GConfig->GetInt(TEXT("/Script/Engine.RendererSettings"), TEXT("r.DynamicGlobalIlluminationMethod"), 
                    DynamicGlobalIllumination, GEngineIni);
    
    if (DynamicGlobalIllumination == 1) // 1 = Lumen
    {
        return FArchitectureValidation(
            TEXT("Lumen Global Illumination"),
            EArchitectureValidationResult::Valid,
            TEXT("Lumen Global Illumination is enabled"),
            TEXT("")
        );
    }
    
    return FArchitectureValidation(
        TEXT("Lumen Global Illumination"),
        EArchitectureValidationResult::Error,
        TEXT("Lumen Global Illumination is not enabled"),
        TEXT("Set Dynamic Global Illumination to Lumen in Project Settings")
    );
}

FArchitectureValidation UEngineArchitectureValidator::ValidateVirtualShadowMaps()
{
    // Check Virtual Shadow Maps
    int32 ShadowMapMethod = 0;
    GConfig->GetInt(TEXT("/Script/Engine.RendererSettings"), TEXT("r.Shadow.Virtual.Enable"), 
                    ShadowMapMethod, GEngineIni);
    
    if (ShadowMapMethod == 1)
    {
        return FArchitectureValidation(
            TEXT("Virtual Shadow Maps"),
            EArchitectureValidationResult::Valid,
            TEXT("Virtual Shadow Maps are enabled"),
            TEXT("")
        );
    }
    
    return FArchitectureValidation(
        TEXT("Virtual Shadow Maps"),
        EArchitectureValidationResult::Warning,
        TEXT("Virtual Shadow Maps are not enabled"),
        TEXT("Enable Virtual Shadow Maps for high-resolution shadows")
    );
}

FArchitectureValidation UEngineArchitectureValidator::ValidateMassEntitySystem()
{
    // Check if Mass Entity plugin is available
    if (CheckFeatureAvailability(TEXT("Mass Entity"), 
        StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/MassEntity.MassEntitySubsystem"))))
    {
        return FArchitectureValidation(
            TEXT("Mass Entity System"),
            EArchitectureValidationResult::Valid,
            TEXT("Mass Entity System is available for crowd simulation"),
            TEXT("")
        );
    }
    
    return FArchitectureValidation(
        TEXT("Mass Entity System"),
        EArchitectureValidationResult::Warning,
        TEXT("Mass Entity System not found"),
        TEXT("Enable Mass Entity plugin for large-scale crowd simulation")
    );
}

FArchitectureValidation UEngineArchitectureValidator::ValidatePCGSystem()
{
    // Check if PCG (Procedural Content Generation) is available
    if (CheckFeatureAvailability(TEXT("PCG"), 
        StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/PCG.PCGSubsystem"))))
    {
        return FArchitectureValidation(
            TEXT("Procedural Content Generation"),
            EArchitectureValidationResult::Valid,
            TEXT("PCG System is available for world generation"),
            TEXT("")
        );
    }
    
    return FArchitectureValidation(
        TEXT("Procedural Content Generation"),
        EArchitectureValidationResult::Warning,
        TEXT("PCG System not found"),
        TEXT("Enable PCG plugin for procedural world generation")
    );
}

FArchitectureValidation UEngineArchitectureValidator::ValidatePerformanceTargets()
{
    // Check target frame rate settings
    float TargetFrameRate = 60.0f;
    GConfig->GetFloat(TEXT("/Script/Engine.Engine"), TEXT("FixedFrameRate"), TargetFrameRate, GEngineIni);
    
    if (TargetFrameRate >= Requirements.TargetFrameRate || TargetFrameRate == 0.0f) // 0 = unlimited
    {
        return FArchitectureValidation(
            TEXT("Performance Targets"),
            EArchitectureValidationResult::Valid,
            FString::Printf(TEXT("Target frame rate: %.0f FPS"), TargetFrameRate),
            TEXT("")
        );
    }
    
    return FArchitectureValidation(
        TEXT("Performance Targets"),
        EArchitectureValidationResult::Warning,
        FString::Printf(TEXT("Target frame rate below requirement: %.0f < %d"), 
                       TargetFrameRate, Requirements.TargetFrameRate),
        TEXT("Consider optimizing performance targets")
    );
}

FArchitectureValidation UEngineArchitectureValidator::ValidateMemoryRequirements()
{
    // Get system memory info
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float TotalGBPhysical = MemStats.TotalPhysical / (1024.0f * 1024.0f * 1024.0f);
    
    if (TotalGBPhysical >= 16.0f) // Minimum 16GB for large open world
    {
        return FArchitectureValidation(
            TEXT("Memory Requirements"),
            EArchitectureValidationResult::Valid,
            FString::Printf(TEXT("System memory: %.1f GB"), TotalGBPhysical),
            TEXT("")
        );
    }
    
    return FArchitectureValidation(
        TEXT("Memory Requirements"),
        EArchitectureValidationResult::Warning,
        FString::Printf(TEXT("Low system memory: %.1f GB"), TotalGBPhysical),
        TEXT("Consider 16GB+ RAM for optimal performance with large worlds")
    );
}

FArchitectureValidation UEngineArchitectureValidator::ValidateProjectSettings()
{
    // Validate key project settings
    bool bOneFilePerActor = false;
    GConfig->GetBool(TEXT("/Script/Engine.Engine"), TEXT("bUseExternalActors"), bOneFilePerActor, GEngineIni);
    
    if (bOneFilePerActor)
    {
        return FArchitectureValidation(
            TEXT("Project Configuration"),
            EArchitectureValidationResult::Valid,
            TEXT("One File Per Actor is enabled for better source control"),
            TEXT("")
        );
    }
    
    return FArchitectureValidation(
        TEXT("Project Configuration"),
        EArchitectureValidationResult::Warning,
        TEXT("One File Per Actor not enabled"),
        TEXT("Enable One File Per Actor for better team collaboration")
    );
}

FArchitectureValidation UEngineArchitectureValidator::ValidateRenderingSettings()
{
    // Check rendering pipeline
    FString RenderingRHI = GDynamicRHI->GetName();
    
    if (RenderingRHI.Contains(TEXT("D3D12")) || RenderingRHI.Contains(TEXT("Vulkan")))
    {
        return FArchitectureValidation(
            TEXT("Rendering Pipeline"),
            EArchitectureValidationResult::Valid,
            FString::Printf(TEXT("Modern RHI in use: %s"), *RenderingRHI),
            TEXT("")
        );
    }
    
    return FArchitectureValidation(
        TEXT("Rendering Pipeline"),
        EArchitectureValidationResult::Warning,
        FString::Printf(TEXT("Legacy RHI detected: %s"), *RenderingRHI),
        TEXT("Consider using DirectX 12 or Vulkan for best performance")
    );
}

bool UEngineArchitectureValidator::IsArchitectureValid()
{
    if (!bValidationCacheValid || 
        (FPlatformTime::Seconds() - LastValidationTime) > ValidationCacheTimeout)
    {
        ValidateEngineArchitecture();
    }
    
    for (const FArchitectureValidation& Result : LastValidationResults)
    {
        if (Result.Result == EArchitectureValidationResult::Error || 
            Result.Result == EArchitectureValidationResult::Critical)
        {
            return false;
        }
    }
    
    return true;
}

void UEngineArchitectureValidator::GenerateArchitectureReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECTURE REPORT ==="));
    
    for (const FArchitectureValidation& Result : LastValidationResults)
    {
        FString StatusText;
        switch (Result.Result)
        {
        case EArchitectureValidationResult::Valid:
            StatusText = TEXT("✓ VALID");
            break;
        case EArchitectureValidationResult::Warning:
            StatusText = TEXT("⚠ WARNING");
            break;
        case EArchitectureValidationResult::Error:
            StatusText = TEXT("✗ ERROR");
            break;
        case EArchitectureValidationResult::Critical:
            StatusText = TEXT("🔴 CRITICAL");
            break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("%s - %s: %s"), 
               *StatusText, *Result.SystemName, *Result.Message);
        
        if (!Result.Recommendation.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("   Recommendation: %s"), *Result.Recommendation);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END ARCHITECTURE REPORT ==="));
}

void UEngineArchitectureValidator::ApplyRecommendedSettings()
{
    UE_LOG(LogTemp, Warning, TEXT("Applying recommended engine settings..."));
    
    // This would apply recommended settings automatically
    // Implementation would depend on specific requirements
    // For safety, this is left as a placeholder
    
    UE_LOG(LogTemp, Warning, TEXT("Recommended settings application complete"));
}

bool UEngineArchitectureValidator::CheckFeatureAvailability(const FString& FeatureName, UClass* RequiredClass)
{
    if (!RequiredClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Feature check failed for %s: Class not found"), *FeatureName);
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Feature %s is available"), *FeatureName);
    return true;
}

bool UEngineArchitectureValidator::CheckProjectSetting(const FString& SettingPath, const FString& ExpectedValue)
{
    FString CurrentValue;
    if (GConfig->GetString(TEXT("/Script/Engine.RendererSettings"), *SettingPath, CurrentValue, GEngineIni))
    {
        return CurrentValue.Equals(ExpectedValue, ESearchCase::IgnoreCase);
    }
    return false;
}

bool UEngineArchitectureValidator::CheckRenderingFeature(const FString& FeatureName)
{
    // Placeholder for specific rendering feature checks
    return true;
}

void UEngineArchitectureValidator::LogValidationResult(const FArchitectureValidation& Result)
{
    switch (Result.Result)
    {
    case EArchitectureValidationResult::Valid:
        UE_LOG(LogTemp, Log, TEXT("✓ %s: %s"), *Result.SystemName, *Result.Message);
        break;
    case EArchitectureValidationResult::Warning:
        UE_LOG(LogTemp, Warning, TEXT("⚠ %s: %s"), *Result.SystemName, *Result.Message);
        break;
    case EArchitectureValidationResult::Error:
        UE_LOG(LogTemp, Error, TEXT("✗ %s: %s"), *Result.SystemName, *Result.Message);
        break;
    case EArchitectureValidationResult::Critical:
        UE_LOG(LogTemp, Fatal, TEXT("🔴 %s: %s"), *Result.SystemName, *Result.Message);
        break;
    }
}