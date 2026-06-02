#include "QA_FinalValidationReport.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"

UQA_FinalValidationReport::UQA_FinalValidationReport()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize validation metrics
    TotalSystemsValidated = 0;
    SystemsPassedValidation = 0;
    CriticalIssuesFound = 0;
    PerformanceIssuesFound = 0;
    
    ValidationStatus = EQA_ValidationStatus::NotStarted;
    OverallBuildStatus = EQA_BuildStatus::Unknown;
}

void UQA_FinalValidationReport::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-start validation on begin play
    StartFinalValidation();
}

void UQA_FinalValidationReport::StartFinalValidation()
{
    ValidationStatus = EQA_ValidationStatus::InProgress;
    ValidationResults.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("QA Final Validation Started"));
    
    // Validate core systems
    ValidateCoreGameSystems();
    ValidateVFXSystems();
    ValidatePerformanceMetrics();
    ValidateMapIntegrity();
    
    // Generate final report
    GenerateFinalReport();
}

void UQA_FinalValidationReport::ValidateCoreGameSystems()
{
    FQA_ValidationResult CoreSystemResult;
    CoreSystemResult.TestName = "Core Game Systems";
    CoreSystemResult.TestCategory = EQA_TestCategory::CoreSystems;
    
    // Test TranspersonalCharacter
    UClass* CharacterClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (CharacterClass)
    {
        CoreSystemResult.Details.Add("TranspersonalCharacter: PASS");
        SystemsPassedValidation++;
    }
    else
    {
        CoreSystemResult.Details.Add("TranspersonalCharacter: FAIL - Class not found");
        CriticalIssuesFound++;
    }
    
    // Test TranspersonalGameState
    UClass* GameStateClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    if (GameStateClass)
    {
        CoreSystemResult.Details.Add("TranspersonalGameState: PASS");
        SystemsPassedValidation++;
    }
    else
    {
        CoreSystemResult.Details.Add("TranspersonalGameState: FAIL - Class not found");
        CriticalIssuesFound++;
    }
    
    // Test PCGWorldGenerator
    UClass* WorldGenClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    if (WorldGenClass)
    {
        CoreSystemResult.Details.Add("PCGWorldGenerator: PASS");
        SystemsPassedValidation++;
    }
    else
    {
        CoreSystemResult.Details.Add("PCGWorldGenerator: FAIL - Class not found");
        CriticalIssuesFound++;
    }
    
    TotalSystemsValidated += 3;
    CoreSystemResult.bPassed = (CriticalIssuesFound == 0);
    ValidationResults.Add(CoreSystemResult);
}

void UQA_FinalValidationReport::ValidateVFXSystems()
{
    FQA_ValidationResult VFXResult;
    VFXResult.TestName = "VFX Systems";
    VFXResult.TestCategory = EQA_TestCategory::VFX;
    
    // Test VFX_NiagaraLibrary
    UClass* VFXClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.VFX_NiagaraLibrary"));
    if (VFXClass)
    {
        VFXResult.Details.Add("VFX_NiagaraLibrary: PASS");
        SystemsPassedValidation++;
    }
    else
    {
        VFXResult.Details.Add("VFX_NiagaraLibrary: FAIL - Class not found");
        CriticalIssuesFound++;
    }
    
    // Check for VFX actors in world
    UWorld* World = GetWorld();
    if (World)
    {
        int32 VFXActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetName().Contains(TEXT("VFX")))
            {
                VFXActorCount++;
            }
        }
        
        if (VFXActorCount > 0)
        {
            VFXResult.Details.Add(FString::Printf(TEXT("VFX Actors in Scene: %d PASS"), VFXActorCount));
        }
        else
        {
            VFXResult.Details.Add("VFX Actors in Scene: 0 FAIL");
            PerformanceIssuesFound++;
        }
    }
    
    TotalSystemsValidated++;
    VFXResult.bPassed = (VFXResult.Details.Num() > 0);
    ValidationResults.Add(VFXResult);
}

void UQA_FinalValidationReport::ValidatePerformanceMetrics()
{
    FQA_ValidationResult PerformanceResult;
    PerformanceResult.TestName = "Performance Metrics";
    PerformanceResult.TestCategory = EQA_TestCategory::Performance;
    
    UWorld* World = GetWorld();
    if (World)
    {
        // Count total actors
        int32 TotalActors = 0;
        int32 DinosaurCount = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                TotalActors++;
                
                FString ActorName = Actor->GetName().ToLower();
                if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
                    ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")))
                {
                    DinosaurCount++;
                }
            }
        }
        
        // Validate actor limits
        if (TotalActors <= 8000)
        {
            PerformanceResult.Details.Add(FString::Printf(TEXT("Total Actors: %d/8000 PASS"), TotalActors));
        }
        else
        {
            PerformanceResult.Details.Add(FString::Printf(TEXT("Total Actors: %d/8000 FAIL - Exceeds limit"), TotalActors));
            PerformanceIssuesFound++;
        }
        
        // Validate dinosaur limits
        if (DinosaurCount <= 150)
        {
            PerformanceResult.Details.Add(FString::Printf(TEXT("Dinosaurs: %d/150 PASS"), DinosaurCount));
        }
        else
        {
            PerformanceResult.Details.Add(FString::Printf(TEXT("Dinosaurs: %d/150 FAIL - Exceeds limit"), DinosaurCount));
            PerformanceIssuesFound++;
        }
    }
    
    TotalSystemsValidated++;
    PerformanceResult.bPassed = (PerformanceIssuesFound == 0);
    ValidationResults.Add(PerformanceResult);
}

void UQA_FinalValidationReport::ValidateMapIntegrity()
{
    FQA_ValidationResult MapResult;
    MapResult.TestName = "Map Integrity";
    MapResult.TestCategory = EQA_TestCategory::MapIntegrity;
    
    UWorld* World = GetWorld();
    if (World)
    {
        // Check for essential actors
        bool bHasPlayerStart = false;
        bool bHasLighting = false;
        int32 QATestZones = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                FString ActorName = Actor->GetName();
                
                if (ActorName.Contains(TEXT("PlayerStart")))
                {
                    bHasPlayerStart = true;
                }
                else if (ActorName.Contains(TEXT("Light")))
                {
                    bHasLighting = true;
                }
                else if (ActorName.Contains(TEXT("QA_")))
                {
                    QATestZones++;
                }
            }
        }
        
        // Validate essential components
        if (bHasPlayerStart)
        {
            MapResult.Details.Add("PlayerStart: PASS");
        }
        else
        {
            MapResult.Details.Add("PlayerStart: FAIL - Not found");
            CriticalIssuesFound++;
        }
        
        if (bHasLighting)
        {
            MapResult.Details.Add("Lighting: PASS");
        }
        else
        {
            MapResult.Details.Add("Lighting: FAIL - No light sources");
            CriticalIssuesFound++;
        }
        
        if (QATestZones >= 4)
        {
            MapResult.Details.Add(FString::Printf(TEXT("QA Test Zones: %d PASS"), QATestZones));
        }
        else
        {
            MapResult.Details.Add(FString::Printf(TEXT("QA Test Zones: %d FAIL - Insufficient"), QATestZones));
            PerformanceIssuesFound++;
        }
    }
    
    TotalSystemsValidated++;
    MapResult.bPassed = (CriticalIssuesFound == 0);
    ValidationResults.Add(MapResult);
}

void UQA_FinalValidationReport::GenerateFinalReport()
{
    ValidationStatus = EQA_ValidationStatus::Completed;
    
    // Determine overall build status
    if (CriticalIssuesFound == 0 && PerformanceIssuesFound <= 2)
    {
        OverallBuildStatus = EQA_BuildStatus::Approved;
    }
    else if (CriticalIssuesFound <= 1 && PerformanceIssuesFound <= 5)
    {
        OverallBuildStatus = EQA_BuildStatus::ConditionalApproval;
    }
    else
    {
        OverallBuildStatus = EQA_BuildStatus::Rejected;
    }
    
    // Log final results
    UE_LOG(LogTemp, Warning, TEXT("=== QA FINAL VALIDATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Systems Validated: %d"), TotalSystemsValidated);
    UE_LOG(LogTemp, Warning, TEXT("Systems Passed: %d"), SystemsPassedValidation);
    UE_LOG(LogTemp, Warning, TEXT("Critical Issues: %d"), CriticalIssuesFound);
    UE_LOG(LogTemp, Warning, TEXT("Performance Issues: %d"), PerformanceIssuesFound);
    
    FString StatusString;
    switch (OverallBuildStatus)
    {
        case EQA_BuildStatus::Approved:
            StatusString = TEXT("APPROVED");
            break;
        case EQA_BuildStatus::ConditionalApproval:
            StatusString = TEXT("CONDITIONAL APPROVAL");
            break;
        case EQA_BuildStatus::Rejected:
            StatusString = TEXT("REJECTED");
            break;
        default:
            StatusString = TEXT("UNKNOWN");
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build Status: %s"), *StatusString);
    UE_LOG(LogTemp, Warning, TEXT("=== END QA REPORT ==="));
}

FString UQA_FinalValidationReport::GetValidationSummary() const
{
    FString Summary = FString::Printf(
        TEXT("QA Validation Summary:\nSystems: %d/%d passed\nCritical Issues: %d\nPerformance Issues: %d\nStatus: %s"),
        SystemsPassedValidation,
        TotalSystemsValidated,
        CriticalIssuesFound,
        PerformanceIssuesFound,
        OverallBuildStatus == EQA_BuildStatus::Approved ? TEXT("APPROVED") : 
        OverallBuildStatus == EQA_BuildStatus::ConditionalApproval ? TEXT("CONDITIONAL") : TEXT("REJECTED")
    );
    
    return Summary;
}

bool UQA_FinalValidationReport::IsBuildApproved() const
{
    return OverallBuildStatus == EQA_BuildStatus::Approved || 
           OverallBuildStatus == EQA_BuildStatus::ConditionalApproval;
}