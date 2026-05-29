#include "Build_IntegrationReport.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "UObject/UObjectGlobals.h"

void UBuild_IntegrationReport::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize core systems to monitor
    CoreSystems.Empty();
    CoreSystems.Add(TPair<FString, FString>(TEXT("Character System"), TEXT("TranspersonalCharacter")));
    CoreSystems.Add(TPair<FString, FString>(TEXT("Game State"), TEXT("TranspersonalGameState")));
    CoreSystems.Add(TPair<FString, FString>(TEXT("World Generation"), TEXT("PCGWorldGenerator")));
    CoreSystems.Add(TPair<FString, FString>(TEXT("Foliage Manager"), TEXT("FoliageManager")));
    CoreSystems.Add(TPair<FString, FString>(TEXT("Crowd Simulation"), TEXT("CrowdSimulationManager")));
    CoreSystems.Add(TPair<FString, FString>(TEXT("Dinosaur AI"), TEXT("DinosaurCombatAIController")));
    CoreSystems.Add(TPair<FString, FString>(TEXT("Dinosaur TRex"), TEXT("DinosaurTRex")));
    CoreSystems.Add(TPair<FString, FString>(TEXT("VFX Manager"), TEXT("VFX_ImpactManager")));
    CoreSystems.Add(TPair<FString, FString>(TEXT("QA Manager"), TEXT("QA_TestManager")));
    
    // Initialize system status
    SystemInfoArray.Empty();
    bAllSystemsOperational = false;
    
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationReport: Initialized with %d core systems"), CoreSystems.Num());
}

void UBuild_IntegrationReport::Deinitialize()
{
    SystemInfoArray.Empty();
    CoreSystems.Empty();
    CompilationErrors.Empty();
    
    Super::Deinitialize();
}

void UBuild_IntegrationReport::ValidateAllSystems()
{
    SystemInfoArray.Empty();
    CompilationErrors.Empty();
    
    int32 OperationalCount = 0;
    
    // Validate each core system
    for (const auto& SystemPair : CoreSystems)
    {
        ValidateSystem(SystemPair.Key, SystemPair.Value);
        
        // Check if system is operational
        if (SystemInfoArray.Num() > 0)
        {
            const FBuild_SystemInfo& LastSystem = SystemInfoArray.Last();
            if (LastSystem.Status == EBuild_SystemStatus::Operational)
            {
                OperationalCount++;
            }
        }
    }
    
    // Update overall status
    bAllSystemsOperational = (OperationalCount == CoreSystems.Num());
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Check cross-system compatibility
    CheckCrossSystemCompatibility();
    
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationReport: Validated %d systems, %d operational"), 
           CoreSystems.Num(), OperationalCount);
}

void UBuild_IntegrationReport::ValidateSystem(const FString& SystemName, const FString& ClassName)
{
    FBuild_SystemInfo SystemInfo;
    SystemInfo.SystemName = SystemName;
    SystemInfo.ClassName = ClassName;
    SystemInfo.Status = EBuild_SystemStatus::Loading;
    
    // Try to load the class
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    if (LoadedClass)
    {
        SystemInfo.bIsLoaded = true;
        SystemInfo.Status = EBuild_SystemStatus::Operational;
        SystemInfo.ErrorMessage = TEXT("");
    }
    else
    {
        SystemInfo.bIsLoaded = false;
        SystemInfo.Status = EBuild_SystemStatus::Failed;
        SystemInfo.ErrorMessage = FString::Printf(TEXT("Failed to load class: %s"), *ClassPath);
        
        // Add to compilation errors
        CompilationErrors.Add(SystemInfo.ErrorMessage);
    }
    
    SystemInfoArray.Add(SystemInfo);
}

TArray<FBuild_SystemInfo> UBuild_IntegrationReport::GetSystemStatus()
{
    return SystemInfoArray;
}

FBuild_PerformanceMetrics UBuild_IntegrationReport::GetPerformanceMetrics()
{
    UpdatePerformanceMetrics();
    return CurrentMetrics;
}

bool UBuild_IntegrationReport::IsSystemOperational(const FString& SystemName)
{
    for (const FBuild_SystemInfo& SystemInfo : SystemInfoArray)
    {
        if (SystemInfo.SystemName == SystemName)
        {
            return SystemInfo.Status == EBuild_SystemStatus::Operational;
        }
    }
    return false;
}

bool UBuild_IntegrationReport::ValidateCharacterVFXIntegration()
{
    bool bCharacterLoaded = IsSystemOperational(TEXT("Character System"));
    bool bVFXLoaded = IsSystemOperational(TEXT("VFX Manager"));
    
    return bCharacterLoaded && bVFXLoaded;
}

bool UBuild_IntegrationReport::ValidateDinosaurAIIntegration()
{
    bool bDinosaurLoaded = IsSystemOperational(TEXT("Dinosaur TRex"));
    bool bAILoaded = IsSystemOperational(TEXT("Dinosaur AI"));
    
    return bDinosaurLoaded && bAILoaded;
}

bool UBuild_IntegrationReport::ValidateWorldFoliageIntegration()
{
    bool bWorldLoaded = IsSystemOperational(TEXT("World Generation"));
    bool bFoliageLoaded = IsSystemOperational(TEXT("Foliage Manager"));
    
    return bWorldLoaded && bFoliageLoaded;
}

bool UBuild_IntegrationReport::CheckCompilationStatus()
{
    // Check if all systems loaded successfully
    return bAllSystemsOperational && CompilationErrors.Num() == 0;
}

TArray<FString> UBuild_IntegrationReport::GetCompilationErrors()
{
    return CompilationErrors;
}

FString UBuild_IntegrationReport::GenerateIntegrationReport()
{
    FString Report;
    FDateTime CurrentTime = FDateTime::Now();
    
    Report += FString::Printf(TEXT("=== TRANSPERSONAL GAME INTEGRATION REPORT ===\n"));
    Report += FString::Printf(TEXT("Generated: %s\n\n"), *CurrentTime.ToString());
    
    // Overall status
    Report += FString::Printf(TEXT("OVERALL STATUS: %s\n"), 
                             bAllSystemsOperational ? TEXT("OPERATIONAL") : TEXT("ISSUES DETECTED"));
    Report += FString::Printf(TEXT("Systems Monitored: %d\n"), CoreSystems.Num());
    
    int32 OperationalCount = 0;
    for (const FBuild_SystemInfo& SystemInfo : SystemInfoArray)
    {
        if (SystemInfo.Status == EBuild_SystemStatus::Operational)
        {
            OperationalCount++;
        }
    }
    Report += FString::Printf(TEXT("Systems Operational: %d/%d\n\n"), OperationalCount, SystemInfoArray.Num());
    
    // System details
    Report += TEXT("=== SYSTEM STATUS ===\n");
    for (const FBuild_SystemInfo& SystemInfo : SystemInfoArray)
    {
        FString StatusText;
        switch (SystemInfo.Status)
        {
            case EBuild_SystemStatus::Operational: StatusText = TEXT("✓ OPERATIONAL"); break;
            case EBuild_SystemStatus::Failed: StatusText = TEXT("✗ FAILED"); break;
            case EBuild_SystemStatus::Loading: StatusText = TEXT("⟳ LOADING"); break;
            case EBuild_SystemStatus::Disabled: StatusText = TEXT("⊘ DISABLED"); break;
            default: StatusText = TEXT("? UNKNOWN"); break;
        }
        
        Report += FString::Printf(TEXT("%-20s: %s\n"), *SystemInfo.SystemName, *StatusText);
        if (!SystemInfo.ErrorMessage.IsEmpty())
        {
            Report += FString::Printf(TEXT("  Error: %s\n"), *SystemInfo.ErrorMessage);
        }
    }
    
    // Performance metrics
    Report += TEXT("\n=== PERFORMANCE METRICS ===\n");
    Report += FString::Printf(TEXT("Total Actors: %d\n"), CurrentMetrics.TotalActors);
    Report += FString::Printf(TEXT("Dinosaur Actors: %d\n"), CurrentMetrics.DinosaurActors);
    Report += FString::Printf(TEXT("VFX Actors: %d\n"), CurrentMetrics.VFXActors);
    Report += FString::Printf(TEXT("Static Mesh Actors: %d\n"), CurrentMetrics.StaticMeshActors);
    
    // Cross-system integration
    Report += TEXT("\n=== CROSS-SYSTEM INTEGRATION ===\n");
    Report += FString::Printf(TEXT("Character-VFX: %s\n"), 
                             ValidateCharacterVFXIntegration() ? TEXT("✓ COMPATIBLE") : TEXT("✗ FAILED"));
    Report += FString::Printf(TEXT("Dinosaur-AI: %s\n"), 
                             ValidateDinosaurAIIntegration() ? TEXT("✓ COMPATIBLE") : TEXT("✗ FAILED"));
    Report += FString::Printf(TEXT("World-Foliage: %s\n"), 
                             ValidateWorldFoliageIntegration() ? TEXT("✓ COMPATIBLE") : TEXT("✗ FAILED"));
    
    // Compilation errors
    if (CompilationErrors.Num() > 0)
    {
        Report += TEXT("\n=== COMPILATION ERRORS ===\n");
        for (const FString& Error : CompilationErrors)
        {
            Report += FString::Printf(TEXT("- %s\n"), *Error);
        }
    }
    
    Report += TEXT("\n=== END REPORT ===\n");
    
    return Report;
}

void UBuild_IntegrationReport::SaveIntegrationReport(const FString& ReportPath)
{
    FString Report = GenerateIntegrationReport();
    
    if (!FFileHelper::SaveStringToFile(Report, *ReportPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Build_IntegrationReport: Failed to save report to %s"), *ReportPath);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Build_IntegrationReport: Report saved to %s"), *ReportPath);
    }
}

void UBuild_IntegrationReport::UpdatePerformanceMetrics()
{
    CurrentMetrics = FBuild_PerformanceMetrics();
    
    // Get world and count actors
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count actors by type
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        CurrentMetrics.TotalActors++;
        
        FString ClassName = Actor->GetClass()->GetName();
        
        if (ClassName.Contains(TEXT("Dinosaur")))
        {
            CurrentMetrics.DinosaurActors++;
        }
        else if (ClassName.Contains(TEXT("VFX")) || ClassName.Contains(TEXT("Niagara")))
        {
            CurrentMetrics.VFXActors++;
        }
        else if (ClassName.Contains(TEXT("StaticMesh")))
        {
            CurrentMetrics.StaticMeshActors++;
        }
    }
}

void UBuild_IntegrationReport::CheckCrossSystemCompatibility()
{
    // Additional cross-system validation logic can be added here
    // For now, we rely on the individual validation functions
}