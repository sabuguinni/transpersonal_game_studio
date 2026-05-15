#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    OverallStatus = EBuild_IntegrationStatus::Pending;
    IntegrationProgress = 0.0f;
    OverallValidationScore = 0.0f;
    MinimumValidationScore = 75.0f;
    MaxValidationErrors = 5;
    bEnableAutomaticValidation = true;
    TotalActorCount = 0;
    ValidatedSystemCount = 0;
    LastFullValidation = FDateTime::Now();
}

void UBuild_FinalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Initializing final integration orchestrator"));
    
    InitializeIntegrationOrchestrator();
}

void UBuild_FinalIntegrationOrchestrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Deinitializing integration orchestrator"));
    
    RegisteredSystems.Empty();
    CrossSystemValidations.Empty();
    
    Super::Deinitialize();
}

void UBuild_FinalIntegrationOrchestrator::InitializeIntegrationOrchestrator()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Starting integration orchestrator initialization"));
    
    // Clear existing data
    RegisteredSystems.Empty();
    CrossSystemValidations.Empty();
    
    // Reset status
    OverallStatus = EBuild_IntegrationStatus::InProgress;
    IntegrationProgress = 0.0f;
    OverallValidationScore = 0.0f;
    
    // Register core systems
    RegisterSystem(TEXT("WorldGeneration"), 0, 0.0f);
    RegisterSystem(TEXT("Environment"), 0, 0.0f);
    RegisterSystem(TEXT("Character"), 0, 0.0f);
    RegisterSystem(TEXT("VFX"), 0, 0.0f);
    RegisterSystem(TEXT("Audio"), 0, 0.0f);
    RegisterSystem(TEXT("Quest"), 0, 0.0f);
    RegisterSystem(TEXT("NPC"), 0, 0.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Integration orchestrator initialized with %d systems"), RegisteredSystems.Num());
}

void UBuild_FinalIntegrationOrchestrator::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Starting comprehensive system validation"));
    
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationOrchestrator: No valid world found for validation"));
        return;
    }
    
    TotalActorCount = 0;
    ValidatedSystemCount = 0;
    
    // Validate each registered system
    for (FBuild_SystemIntegrationData& SystemData : RegisteredSystems)
    {
        if (ValidateSystemIntegrity(SystemData.SystemName))
        {
            SystemData.Status = EBuild_IntegrationStatus::Validated;
            SystemData.LastValidation = FDateTime::Now();
            ValidatedSystemCount++;
            
            UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: System '%s' validated successfully"), *SystemData.SystemName);
        }
        else
        {
            SystemData.Status = EBuild_IntegrationStatus::Failed;
            UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationOrchestrator: System '%s' validation failed"), *SystemData.SystemName);
        }
        
        TotalActorCount += SystemData.ActorCount;
    }
    
    // Update overall progress
    UpdateIntegrationProgress();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: System validation complete - %d/%d systems validated"), ValidatedSystemCount, RegisteredSystems.Num());
}

void UBuild_FinalIntegrationOrchestrator::PerformCrossSystemValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Starting cross-system validation"));
    
    CrossSystemValidations.Empty();
    
    // Validate critical system pairs
    ValidateSystemCompatibility(TEXT("Character"), TEXT("Environment"));
    ValidateSystemCompatibility(TEXT("VFX"), TEXT("Audio"));
    ValidateSystemCompatibility(TEXT("Quest"), TEXT("NPC"));
    ValidateSystemCompatibility(TEXT("WorldGeneration"), TEXT("Environment"));
    ValidateSystemCompatibility(TEXT("Character"), TEXT("Quest"));
    
    // Calculate overall compatibility score
    float TotalCompatibilityScore = 0.0f;
    int32 ValidCompatibilities = 0;
    
    for (const FBuild_CrossSystemValidation& Validation : CrossSystemValidations)
    {
        TotalCompatibilityScore += Validation.CompatibilityScore;
        if (Validation.bIsCompatible)
        {
            ValidCompatibilities++;
        }
    }
    
    float AverageCompatibility = CrossSystemValidations.Num() > 0 ? TotalCompatibilityScore / CrossSystemValidations.Num() : 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Cross-system validation complete - %d/%d compatible, average score: %.2f"), 
           ValidCompatibilities, CrossSystemValidations.Num(), AverageCompatibility);
}

void UBuild_FinalIntegrationOrchestrator::GenerateFinalBuildReport()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Generating final build report"));
    
    float OverallScore = CalculateOverallIntegrationScore();
    TArray<FString> ValidationErrors = GetSystemValidationErrors();
    
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL BUILD INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Overall Integration Score: %.2f"), OverallScore);
    UE_LOG(LogTemp, Warning, TEXT("Total Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Validated Systems: %d"), ValidatedSystemCount);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), TotalActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Integration Progress: %.2f%%"), IntegrationProgress);
    UE_LOG(LogTemp, Warning, TEXT("Cross-System Validations: %d"), CrossSystemValidations.Num());
    
    if (ValidationErrors.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Validation Errors Found: %d"), ValidationErrors.Num());
        for (const FString& Error : ValidationErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("- %s"), *Error);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No validation errors found"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END BUILD REPORT ==="));
    
    LastFullValidation = FDateTime::Now();
}

void UBuild_FinalIntegrationOrchestrator::RegisterSystem(const FString& SystemName, int32 ActorCount, float ValidationScore)
{
    // Check if system already exists
    FBuild_SystemIntegrationData* ExistingSystem = FindSystemData(SystemName);
    if (ExistingSystem)
    {
        ExistingSystem->ActorCount = ActorCount;
        ExistingSystem->ValidationScore = ValidationScore;
        ExistingSystem->LastValidation = FDateTime::Now();
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Updated existing system '%s'"), *SystemName);
        return;
    }
    
    // Create new system data
    FBuild_SystemIntegrationData NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.ActorCount = ActorCount;
    NewSystem.ValidationScore = ValidationScore;
    NewSystem.Status = EBuild_IntegrationStatus::Pending;
    NewSystem.LastValidation = FDateTime::Now();
    
    RegisteredSystems.Add(NewSystem);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Registered new system '%s' with %d actors"), *SystemName, ActorCount);
}

void UBuild_FinalIntegrationOrchestrator::UnregisterSystem(const FString& SystemName)
{
    RegisteredSystems.RemoveAll([SystemName](const FBuild_SystemIntegrationData& System)
    {
        return System.SystemName == SystemName;
    });
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Unregistered system '%s'"), *SystemName);
}

bool UBuild_FinalIntegrationOrchestrator::ValidateSystemIntegrity(const FString& SystemName)
{
    FBuild_SystemIntegrationData* SystemData = FindSystemData(SystemName);
    if (!SystemData)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationOrchestrator: System '%s' not found for validation"), *SystemName);
        return false;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationOrchestrator: No valid world for system validation"));
        return false;
    }
    
    // Count actors related to this system
    int32 SystemActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(SystemName))
        {
            SystemActorCount++;
        }
    }
    
    // Update system data
    SystemData->ActorCount = SystemActorCount;
    SystemData->ValidationScore = SystemActorCount > 0 ? 100.0f : 0.0f;
    
    bool bIsValid = SystemActorCount > 0 || SystemName == TEXT("WorldGeneration");
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: System '%s' validation - %d actors, valid: %s"), 
           *SystemName, SystemActorCount, bIsValid ? TEXT("YES") : TEXT("NO"));
    
    return bIsValid;
}

float UBuild_FinalIntegrationOrchestrator::CalculateOverallIntegrationScore()
{
    if (RegisteredSystems.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalScore = 0.0f;
    int32 ValidSystems = 0;
    
    for (const FBuild_SystemIntegrationData& System : RegisteredSystems)
    {
        if (System.Status == EBuild_IntegrationStatus::Validated || System.Status == EBuild_IntegrationStatus::Completed)
        {
            TotalScore += System.ValidationScore;
            ValidSystems++;
        }
    }
    
    OverallValidationScore = ValidSystems > 0 ? TotalScore / ValidSystems : 0.0f;
    return OverallValidationScore;
}

TArray<FString> UBuild_FinalIntegrationOrchestrator::GetSystemValidationErrors()
{
    TArray<FString> Errors;
    
    for (const FBuild_SystemIntegrationData& System : RegisteredSystems)
    {
        if (System.Status == EBuild_IntegrationStatus::Failed)
        {
            Errors.Add(FString::Printf(TEXT("System '%s' validation failed"), *System.SystemName));
        }
        else if (System.ValidationScore < MinimumValidationScore)
        {
            Errors.Add(FString::Printf(TEXT("System '%s' score %.2f below minimum %.2f"), 
                      *System.SystemName, System.ValidationScore, MinimumValidationScore));
        }
    }
    
    return Errors;
}

void UBuild_FinalIntegrationOrchestrator::ValidateSystemCompatibility(const FString& SystemA, const FString& SystemB)
{
    FBuild_CrossSystemValidation Validation;
    Validation.SystemA = SystemA;
    Validation.SystemB = SystemB;
    
    // Find system data
    FBuild_SystemIntegrationData* DataA = FindSystemData(SystemA);
    FBuild_SystemIntegrationData* DataB = FindSystemData(SystemB);
    
    if (!DataA || !DataB)
    {
        Validation.bIsCompatible = false;
        Validation.CompatibilityScore = 0.0f;
        Validation.ValidationErrors.Add(TEXT("One or both systems not found"));
    }
    else
    {
        // Simple compatibility check based on validation scores
        float AverageScore = (DataA->ValidationScore + DataB->ValidationScore) / 2.0f;
        Validation.CompatibilityScore = AverageScore;
        Validation.bIsCompatible = AverageScore >= MinimumValidationScore;
        
        if (!Validation.bIsCompatible)
        {
            Validation.ValidationErrors.Add(FString::Printf(TEXT("Low compatibility score: %.2f"), AverageScore));
        }
    }
    
    CrossSystemValidations.Add(Validation);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Validated compatibility between '%s' and '%s' - Compatible: %s, Score: %.2f"), 
           *SystemA, *SystemB, Validation.bIsCompatible ? TEXT("YES") : TEXT("NO"), Validation.CompatibilityScore);
}

bool UBuild_FinalIntegrationOrchestrator::AreSystemsCompatible(const FString& SystemA, const FString& SystemB)
{
    for (const FBuild_CrossSystemValidation& Validation : CrossSystemValidations)
    {
        if ((Validation.SystemA == SystemA && Validation.SystemB == SystemB) ||
            (Validation.SystemA == SystemB && Validation.SystemB == SystemA))
        {
            return Validation.bIsCompatible;
        }
    }
    
    return false;
}

void UBuild_FinalIntegrationOrchestrator::OrchestrateFinalBuild()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Starting final build orchestration"));
    
    OverallStatus = EBuild_IntegrationStatus::InProgress;
    
    // Step 1: Validate all systems
    ValidateAllSystems();
    
    // Step 2: Perform cross-system validation
    PerformCrossSystemValidation();
    
    // Step 3: Generate integration metrics
    GenerateIntegrationMetrics();
    
    // Step 4: Validate build integrity
    ValidateBuildIntegrity();
    
    // Step 5: Generate final report
    GenerateFinalBuildReport();
    
    // Determine final status
    float OverallScore = CalculateOverallIntegrationScore();
    TArray<FString> Errors = GetSystemValidationErrors();
    
    if (OverallScore >= MinimumValidationScore && Errors.Num() <= MaxValidationErrors)
    {
        OverallStatus = EBuild_IntegrationStatus::Completed;
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Final build orchestration COMPLETED successfully"));
    }
    else
    {
        OverallStatus = EBuild_IntegrationStatus::Failed;
        UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationOrchestrator: Final build orchestration FAILED - Score: %.2f, Errors: %d"), 
               OverallScore, Errors.Num());
    }
}

void UBuild_FinalIntegrationOrchestrator::ValidateBuildIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Validating build integrity"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationOrchestrator: No valid world for build integrity validation"));
        return;
    }
    
    // Count total actors
    int32 WorldActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        if (*ActorItr)
        {
            WorldActorCount++;
        }
    }
    
    TotalActorCount = WorldActorCount;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Build integrity validation complete - %d total actors"), TotalActorCount);
}

void UBuild_FinalIntegrationOrchestrator::GenerateIntegrationMetrics()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Generating integration metrics"));
    
    // Update integration progress
    UpdateIntegrationProgress();
    
    // Log detailed metrics
    UE_LOG(LogTemp, Warning, TEXT("Integration Metrics:"));
    UE_LOG(LogTemp, Warning, TEXT("- Registered Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("- Validated Systems: %d"), ValidatedSystemCount);
    UE_LOG(LogTemp, Warning, TEXT("- Integration Progress: %.2f%%"), IntegrationProgress);
    UE_LOG(LogTemp, Warning, TEXT("- Overall Validation Score: %.2f"), OverallValidationScore);
    UE_LOG(LogTemp, Warning, TEXT("- Total Actor Count: %d"), TotalActorCount);
    UE_LOG(LogTemp, Warning, TEXT("- Cross-System Validations: %d"), CrossSystemValidations.Num());
}

void UBuild_FinalIntegrationOrchestrator::UpdateIntegrationProgress()
{
    if (RegisteredSystems.Num() == 0)
    {
        IntegrationProgress = 0.0f;
        return;
    }
    
    int32 CompletedSystems = 0;
    for (const FBuild_SystemIntegrationData& System : RegisteredSystems)
    {
        if (System.Status == EBuild_IntegrationStatus::Validated || System.Status == EBuild_IntegrationStatus::Completed)
        {
            CompletedSystems++;
        }
    }
    
    IntegrationProgress = (float)CompletedSystems / RegisteredSystems.Num() * 100.0f;
}

void UBuild_FinalIntegrationOrchestrator::LogIntegrationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Current Status - %s, Progress: %.2f%%, Score: %.2f"), 
           *UEnum::GetValueAsString(OverallStatus), IntegrationProgress, OverallValidationScore);
}

FBuild_SystemIntegrationData* UBuild_FinalIntegrationOrchestrator::FindSystemData(const FString& SystemName)
{
    for (FBuild_SystemIntegrationData& System : RegisteredSystems)
    {
        if (System.SystemName == SystemName)
        {
            return &System;
        }
    }
    return nullptr;
}