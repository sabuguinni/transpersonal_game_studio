#include "Core_ArchitecturalCompliance.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UCore_ArchitecturalCompliance::UCore_ArchitecturalCompliance()
{
    MinPerformanceThreshold = 30.0f;
    MaxPerformanceThreshold = 100.0f;
    bAutoValidateOnRegistration = true;
    ValidationInterval = 60.0f;
    bStrictComplianceMode = true;
    bIsInitialized = false;
    TotalSystemsRegistered = 0;
    CompliantSystemsCount = 0;
    GovernanceSystem = nullptr;
}

void UCore_ArchitecturalCompliance::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_ArchitecturalCompliance: Initializing subsystem"));
    
    // Initialize default core systems
    InitializeDefaultCoreSystems();
    
    // Attempt to integrate with governance system
    IntegrateWithGovernanceSystem();
    
    // Start periodic validation timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            [this]() { ValidateAllCoreSystems(); },
            ValidationInterval,
            true
        );
    }
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Core_ArchitecturalCompliance: Initialization complete"));
}

void UCore_ArchitecturalCompliance::Deinitialize()
{
    if (GetWorld() && ValidationTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    SystemComplianceMap.Empty();
    GovernanceSystem = nullptr;
    bIsInitialized = false;
    
    Super::Deinitialize();
}

bool UCore_ArchitecturalCompliance::RegisterCoreSystem(const FString& SystemName, EEng_ArchitecturalLayer Layer)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Core_ArchitecturalCompliance: Cannot register system with empty name"));
        return false;
    }
    
    FCore_SystemComplianceData ComplianceData;
    ComplianceData.SystemName = SystemName;
    ComplianceData.AssignedLayer = Layer;
    ComplianceData.ComplianceStatus = ECore_ComplianceStatus::Pending;
    ComplianceData.PerformanceScore = 50.0f; // Default middle score
    ComplianceData.LastValidation = FDateTime::Now();
    
    SystemComplianceMap.Add(SystemName, ComplianceData);
    TotalSystemsRegistered++;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_ArchitecturalCompliance: Registered system %s at layer %d"), 
           *SystemName, (int32)Layer);
    
    // Auto-validate if enabled
    if (bAutoValidateOnRegistration)
    {
        ValidateSystemCompliance(SystemName);
    }
    
    // Trigger Blueprint event
    OnSystemRegistered(SystemName, Layer);
    
    return true;
}

bool UCore_ArchitecturalCompliance::UnregisterCoreSystem(const FString& SystemName)
{
    if (SystemComplianceMap.Contains(SystemName))
    {
        FCore_SystemComplianceData* ComplianceData = SystemComplianceMap.Find(SystemName);
        if (ComplianceData && ComplianceData->ComplianceStatus == ECore_ComplianceStatus::Compliant)
        {
            CompliantSystemsCount--;
        }
        
        SystemComplianceMap.Remove(SystemName);
        TotalSystemsRegistered--;
        
        UE_LOG(LogTemp, Warning, TEXT("Core_ArchitecturalCompliance: Unregistered system %s"), *SystemName);
        return true;
    }
    
    return false;
}

ECore_ComplianceStatus UCore_ArchitecturalCompliance::ValidateSystemCompliance(const FString& SystemName)
{
    FCore_SystemComplianceData* ComplianceData = SystemComplianceMap.Find(SystemName);
    if (!ComplianceData)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_ArchitecturalCompliance: System %s not found for validation"), *SystemName);
        return ECore_ComplianceStatus::Unknown;
    }
    
    // Clear previous violations
    ComplianceData->ViolationReasons.Empty();
    
    // Validate system architecture
    ValidateSystemArchitecture(SystemName, *ComplianceData);
    
    // Check performance compliance
    CheckPerformanceCompliance(SystemName, *ComplianceData);
    
    // Determine final compliance status
    if (ComplianceData->ViolationReasons.Num() == 0)
    {
        ComplianceData->ComplianceStatus = ECore_ComplianceStatus::Compliant;
        CompliantSystemsCount++;
    }
    else
    {
        ComplianceData->ComplianceStatus = ECore_ComplianceStatus::NonCompliant;
        
        // Report violations
        for (const FString& Reason : ComplianceData->ViolationReasons)
        {
            ReportComplianceViolation(SystemName, Reason);
        }
    }
    
    ComplianceData->LastValidation = FDateTime::Now();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_ArchitecturalCompliance: System %s validation result: %d"), 
           *SystemName, (int32)ComplianceData->ComplianceStatus);
    
    return ComplianceData->ComplianceStatus;
}

bool UCore_ArchitecturalCompliance::ValidateAllCoreSystems()
{
    if (!bIsInitialized)
    {
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_ArchitecturalCompliance: Validating all core systems"));
    
    CompliantSystemsCount = 0;
    
    for (auto& SystemPair : SystemComplianceMap)
    {
        ValidateSystemCompliance(SystemPair.Key);
    }
    
    float CompliancePercentage = TotalSystemsRegistered > 0 ? 
        (float)CompliantSystemsCount / (float)TotalSystemsRegistered * 100.0f : 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_ArchitecturalCompliance: Overall compliance: %.1f%% (%d/%d systems)"), 
           CompliancePercentage, CompliantSystemsCount, TotalSystemsRegistered);
    
    return CompliancePercentage >= 80.0f; // 80% compliance threshold
}

float UCore_ArchitecturalCompliance::GetSystemPerformanceScore(const FString& SystemName)
{
    FCore_SystemComplianceData* ComplianceData = SystemComplianceMap.Find(SystemName);
    return ComplianceData ? ComplianceData->PerformanceScore : 0.0f;
}

void UCore_ArchitecturalCompliance::UpdateSystemPerformanceScore(const FString& SystemName, float Score)
{
    FCore_SystemComplianceData* ComplianceData = SystemComplianceMap.Find(SystemName);
    if (ComplianceData)
    {
        ComplianceData->PerformanceScore = FMath::Clamp(Score, 0.0f, 100.0f);
        
        if (Score > MaxPerformanceThreshold)
        {
            OnPerformanceThresholdExceeded(SystemName, Score);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Core_ArchitecturalCompliance: Updated performance score for %s: %.1f"), 
               *SystemName, Score);
    }
}

bool UCore_ArchitecturalCompliance::IntegrateWithGovernanceSystem()
{
    if (!GovernanceSystem)
    {
        // Try to get governance subsystem
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            GovernanceSystem = GameInstance->GetSubsystem<UEng_ArchitecturalGovernance>();
        }
    }
    
    if (GovernanceSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_ArchitecturalCompliance: Successfully integrated with governance system"));
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_ArchitecturalCompliance: Governance system not available"));
    return false;
}

bool UCore_ArchitecturalCompliance::CanSystemActivate(const FString& SystemName)
{
    FCore_SystemComplianceData* ComplianceData = SystemComplianceMap.Find(SystemName);
    if (!ComplianceData)
    {
        return false; // Unknown systems cannot activate
    }
    
    // Check compliance status
    if (bStrictComplianceMode && ComplianceData->ComplianceStatus != ECore_ComplianceStatus::Compliant)
    {
        return false;
    }
    
    // Check performance threshold
    if (ComplianceData->PerformanceScore < MinPerformanceThreshold)
    {
        return false;
    }
    
    // Check with governance system if available
    if (GovernanceSystem)
    {
        // Governance system integration would go here
        // For now, assume governance allows activation
    }
    
    return true;
}

TArray<FCore_SystemComplianceData> UCore_ArchitecturalCompliance::GetComplianceReport()
{
    TArray<FCore_SystemComplianceData> Report;
    
    for (const auto& SystemPair : SystemComplianceMap)
    {
        Report.Add(SystemPair.Value);
    }
    
    return Report;
}

FString UCore_ArchitecturalCompliance::GenerateComplianceReport()
{
    FString Report = TEXT("=== CORE SYSTEMS ARCHITECTURAL COMPLIANCE REPORT ===\n\n");
    
    Report += FString::Printf(TEXT("Total Systems Registered: %d\n"), TotalSystemsRegistered);
    Report += FString::Printf(TEXT("Compliant Systems: %d\n"), CompliantSystemsCount);
    
    float CompliancePercentage = TotalSystemsRegistered > 0 ? 
        (float)CompliantSystemsCount / (float)TotalSystemsRegistered * 100.0f : 0.0f;
    Report += FString::Printf(TEXT("Overall Compliance: %.1f%%\n\n"), CompliancePercentage);
    
    Report += TEXT("SYSTEM DETAILS:\n");
    
    for (const auto& SystemPair : SystemComplianceMap)
    {
        const FCore_SystemComplianceData& Data = SystemPair.Value;
        Report += FString::Printf(TEXT("- %s: %s (Score: %.1f)\n"), 
                                  *Data.SystemName,
                                  *UEnum::GetValueAsString(Data.ComplianceStatus),
                                  Data.PerformanceScore);
        
        if (Data.ViolationReasons.Num() > 0)
        {
            Report += TEXT("  Violations: ");
            for (const FString& Reason : Data.ViolationReasons)
            {
                Report += FString::Printf(TEXT("%s; "), *Reason);
            }
            Report += TEXT("\n");
        }
    }
    
    return Report;
}

void UCore_ArchitecturalCompliance::ValidateSystemArchitecture(const FString& SystemName, FCore_SystemComplianceData& ComplianceData)
{
    // Validate architectural layer compliance
    switch (ComplianceData.AssignedLayer)
    {
        case EEng_ArchitecturalLayer::Foundation:
            // Foundation systems should have minimal dependencies
            if (SystemName.Contains(TEXT("Manager")) || SystemName.Contains(TEXT("Controller")))
            {
                ComplianceData.ViolationReasons.Add(TEXT("Foundation layer should not contain management logic"));
            }
            break;
            
        case EEng_ArchitecturalLayer::Core:
            // Core systems should build on foundation
            if (!SystemName.Contains(TEXT("Core_")))
            {
                ComplianceData.ViolationReasons.Add(TEXT("Core layer systems must use Core_ prefix"));
            }
            break;
            
        case EEng_ArchitecturalLayer::Gameplay:
            // Gameplay systems should not directly access foundation
            break;
            
        default:
            ComplianceData.ViolationReasons.Add(TEXT("Invalid architectural layer assignment"));
            break;
    }
}

void UCore_ArchitecturalCompliance::CheckPerformanceCompliance(const FString& SystemName, FCore_SystemComplianceData& ComplianceData)
{
    // Check performance thresholds
    if (ComplianceData.PerformanceScore < MinPerformanceThreshold)
    {
        ComplianceData.ViolationReasons.Add(FString::Printf(TEXT("Performance below minimum threshold (%.1f < %.1f)"), 
                                                             ComplianceData.PerformanceScore, MinPerformanceThreshold));
    }
    
    if (ComplianceData.PerformanceScore > MaxPerformanceThreshold)
    {
        ComplianceData.ViolationReasons.Add(FString::Printf(TEXT("Performance above maximum threshold (%.1f > %.1f)"), 
                                                             ComplianceData.PerformanceScore, MaxPerformanceThreshold));
    }
}

void UCore_ArchitecturalCompliance::ReportComplianceViolation(const FString& SystemName, const FString& Reason)
{
    UE_LOG(LogTemp, Error, TEXT("Core_ArchitecturalCompliance: VIOLATION - System %s: %s"), *SystemName, *Reason);
    
    // Trigger Blueprint event
    OnComplianceViolation(SystemName, Reason);
}

void UCore_ArchitecturalCompliance::InitializeDefaultCoreSystems()
{
    // Register core physics systems with appropriate architectural layers
    RegisterCoreSystem(TEXT("Core_PhysicsCore"), EEng_ArchitecturalLayer::Foundation);
    RegisterCoreSystem(TEXT("Core_CollisionManager"), EEng_ArchitecturalLayer::Foundation);
    RegisterCoreSystem(TEXT("Core_PhysicsSystemCoordinator"), EEng_ArchitecturalLayer::Core);
    RegisterCoreSystem(TEXT("Core_PhysicsManager"), EEng_ArchitecturalLayer::Core);
    RegisterCoreSystem(TEXT("Core_RagdollSystem"), EEng_ArchitecturalLayer::Core);
    RegisterCoreSystem(TEXT("Core_DestructionSystem"), EEng_ArchitecturalLayer::Core);
    RegisterCoreSystem(TEXT("Core_PhysicsValidation"), EEng_ArchitecturalLayer::Core);
    RegisterCoreSystem(TEXT("Core_MovementSystem"), EEng_ArchitecturalLayer::Gameplay);
    RegisterCoreSystem(TEXT("Core_CharacterPhysics"), EEng_ArchitecturalLayer::Gameplay);
    RegisterCoreSystem(TEXT("Core_TerrainPhysics"), EEng_ArchitecturalLayer::Gameplay);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_ArchitecturalCompliance: Initialized %d default core systems"), TotalSystemsRegistered);
}