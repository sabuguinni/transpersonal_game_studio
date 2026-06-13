#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/PlayerStart.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    MaxActorCount = 8000;
    MaxDinosaurCount = 150;
    bAutoFixActorCounts = true;
    bVerboseLogging = true;
}

void UBuild_IntegrationValidator::RunFullValidation()
{
    ClearValidationReports();
    
    double StartTime = FPlatformTime::Seconds();
    
    ValidateActorCounts();
    ValidateEssentialSystems();
    ValidateModuleCompilation();
    ValidateQAIntegration();
    ValidateBuildSystem();
    
    double TotalTime = FPlatformTime::Seconds() - StartTime;
    
    FString SummaryDetails = FString::Printf(TEXT("Total validation time: %.3f seconds. %d tests completed."), 
        TotalTime, ValidationReports.Num());
    
    AddValidationReport(TEXT("Full Validation"), 
        IsSystemHealthy() ? EBuild_ValidationResult::Pass : EBuild_ValidationResult::Fail,
        SummaryDetails, TotalTime);
    
    UE_LOG(LogTemp, Warning, TEXT("Integration Validation Complete: %s"), 
        IsSystemHealthy() ? TEXT("HEALTHY") : TEXT("ISSUES DETECTED"));
}

void UBuild_IntegrationValidator::ValidateActorCounts()
{
    double StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("Actor Count Validation"), EBuild_ValidationResult::Fail, 
            TEXT("No valid world found"), FPlatformTime::Seconds() - StartTime);
        return;
    }
    
    TArray<AActor*> AllActors;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AllActors.Add(*ActorItr);
    }
    
    int32 ActorCount = AllActors.Num();
    
    // Count dinosaurs
    TArray<FString> DinoLabels = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), 
        TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")};
    TArray<AActor*> Dinosaurs = GetActorsOfType(DinoLabels);
    
    EBuild_ValidationResult Result = EBuild_ValidationResult::Pass;
    FString Details;
    
    if (ActorCount > MaxActorCount)
    {
        Result = EBuild_ValidationResult::Warning;
        Details = FString::Printf(TEXT("Actor count (%d) exceeds maximum (%d). Dinosaurs: %d"), 
            ActorCount, MaxActorCount, Dinosaurs.Num());
    }
    else if (Dinosaurs.Num() > MaxDinosaurCount)
    {
        Result = EBuild_ValidationResult::Warning;
        Details = FString::Printf(TEXT("Dinosaur count (%d) exceeds maximum (%d). Total actors: %d"), 
            Dinosaurs.Num(), MaxDinosaurCount, ActorCount);
    }
    else
    {
        Details = FString::Printf(TEXT("Actor count: %d (within limits). Dinosaurs: %d"), 
            ActorCount, Dinosaurs.Num());
    }
    
    AddValidationReport(TEXT("Actor Count Validation"), Result, Details, 
        FPlatformTime::Seconds() - StartTime);
}

void UBuild_IntegrationValidator::ValidateEssentialSystems()
{
    double StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("Essential Systems"), EBuild_ValidationResult::Fail, 
            TEXT("No valid world found"), FPlatformTime::Seconds() - StartTime);
        return;
    }
    
    TMap<FString, bool> EssentialSystems;
    EssentialSystems.Add(TEXT("PlayerStart"), false);
    EssentialSystems.Add(TEXT("DirectionalLight"), false);
    EssentialSystems.Add(TEXT("SkyLight"), false);
    EssentialSystems.Add(TEXT("SkyAtmosphere"), false);
    EssentialSystems.Add(TEXT("ExponentialHeightFog"), false);
    
    // Check for PlayerStart
    for (TActorIterator<APlayerStart> PlayerStartItr(World); PlayerStartItr; ++PlayerStartItr)
    {
        EssentialSystems[TEXT("PlayerStart")] = true;
        break;
    }
    
    // Check for lighting components
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor->FindComponentByClass<UDirectionalLightComponent>())
        {
            EssentialSystems[TEXT("DirectionalLight")] = true;
        }
        if (Actor->FindComponentByClass<USkyLightComponent>())
        {
            EssentialSystems[TEXT("SkyLight")] = true;
        }
        if (Actor->FindComponentByClass<USkyAtmosphereComponent>())
        {
            EssentialSystems[TEXT("SkyAtmosphere")] = true;
        }
        if (Actor->FindComponentByClass<UExponentialHeightFogComponent>())
        {
            EssentialSystems[TEXT("ExponentialHeightFog")] = true;
        }
    }
    
    int32 MissingSystems = 0;
    FString Details = TEXT("Essential systems status: ");
    
    for (const auto& System : EssentialSystems)
    {
        Details += FString::Printf(TEXT("%s: %s, "), 
            *System.Key, System.Value ? TEXT("OK") : TEXT("MISSING"));
        if (!System.Value) MissingSystems++;
    }
    
    EBuild_ValidationResult Result = (MissingSystems == 0) ? EBuild_ValidationResult::Pass : 
        (MissingSystems <= 2) ? EBuild_ValidationResult::Warning : EBuild_ValidationResult::Fail;
    
    AddValidationReport(TEXT("Essential Systems"), Result, Details, 
        FPlatformTime::Seconds() - StartTime);
}

void UBuild_IntegrationValidator::ValidateModuleCompilation()
{
    double StartTime = FPlatformTime::Seconds();
    
    TArray<FString> CoreClasses = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager")
    };
    
    int32 LoadedClasses = 0;
    FString Details = TEXT("Module compilation status: ");
    
    for (const FString& ClassName : CoreClasses)
    {
        bool bLoaded = ValidateClassLoading(ClassName);
        Details += FString::Printf(TEXT("%s: %s, "), 
            *ClassName, bLoaded ? TEXT("OK") : TEXT("FAIL"));
        if (bLoaded) LoadedClasses++;
    }
    
    EBuild_ValidationResult Result = (LoadedClasses == CoreClasses.Num()) ? EBuild_ValidationResult::Pass :
        (LoadedClasses >= CoreClasses.Num() / 2) ? EBuild_ValidationResult::Warning : EBuild_ValidationResult::Fail;
    
    AddValidationReport(TEXT("Module Compilation"), Result, Details, 
        FPlatformTime::Seconds() - StartTime);
}

void UBuild_IntegrationValidator::ValidateQAIntegration()
{
    double StartTime = FPlatformTime::Seconds();
    
    TArray<FString> QAClasses = {
        TEXT("QA_VFXTestSuite"),
        TEXT("QA_AudioTestFramework")
    };
    
    int32 LoadedQAClasses = 0;
    FString Details = TEXT("QA integration status: ");
    
    for (const FString& ClassName : QAClasses)
    {
        bool bLoaded = ValidateClassLoading(ClassName);
        Details += FString::Printf(TEXT("%s: %s, "), 
            *ClassName, bLoaded ? TEXT("OK") : TEXT("FAIL"));
        if (bLoaded) LoadedQAClasses++;
    }
    
    EBuild_ValidationResult Result = (LoadedQAClasses > 0) ? EBuild_ValidationResult::Pass : EBuild_ValidationResult::Warning;
    
    AddValidationReport(TEXT("QA Integration"), Result, Details, 
        FPlatformTime::Seconds() - StartTime);
}

void UBuild_IntegrationValidator::ValidateBuildSystem()
{
    double StartTime = FPlatformTime::Seconds();
    
    // Basic build system validation - check if we can access project paths
    FString ProjectPath = FPaths::GetProjectFilePath();
    bool bProjectPathValid = !ProjectPath.IsEmpty() && FPaths::FileExists(ProjectPath);
    
    FString Details = FString::Printf(TEXT("Project path valid: %s"), 
        bProjectPathValid ? TEXT("YES") : TEXT("NO"));
    
    EBuild_ValidationResult Result = bProjectPathValid ? EBuild_ValidationResult::Pass : EBuild_ValidationResult::Fail;
    
    AddValidationReport(TEXT("Build System"), Result, Details, 
        FPlatformTime::Seconds() - StartTime);
}

bool UBuild_IntegrationValidator::IsSystemHealthy() const
{
    for (const FBuild_ValidationReport& Report : ValidationReports)
    {
        if (Report.Result == EBuild_ValidationResult::Fail)
        {
            return false;
        }
    }
    return true;
}

void UBuild_IntegrationValidator::ClearValidationReports()
{
    ValidationReports.Empty();
}

void UBuild_IntegrationValidator::AddValidationReport(const FString& TestName, EBuild_ValidationResult Result, const FString& Details, float ExecutionTime)
{
    FBuild_ValidationReport Report;
    Report.TestName = TestName;
    Report.Result = Result;
    Report.Details = Details;
    Report.ExecutionTime = ExecutionTime;
    
    ValidationReports.Add(Report);
    LogValidationResult(TestName, Result, Details);
}

void UBuild_IntegrationValidator::LogValidationResult(const FString& TestName, EBuild_ValidationResult Result, const FString& Details)
{
    if (!bVerboseLogging) return;
    
    FString ResultString;
    switch (Result)
    {
        case EBuild_ValidationResult::Pass: ResultString = TEXT("PASS"); break;
        case EBuild_ValidationResult::Fail: ResultString = TEXT("FAIL"); break;
        case EBuild_ValidationResult::Warning: ResultString = TEXT("WARN"); break;
        default: ResultString = TEXT("UNKNOWN"); break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VALIDATION [%s] %s: %s"), *ResultString, *TestName, *Details);
}

TArray<AActor*> UBuild_IntegrationValidator::GetActorsOfType(const TArray<FString>& TypeLabels)
{
    TArray<AActor*> FoundActors;
    UWorld* World = GetWorld();
    if (!World) return FoundActors;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        for (const FString& Label : TypeLabels)
        {
            if (ActorLabel.Contains(Label.ToLower()))
            {
                FoundActors.Add(Actor);
                break;
            }
        }
    }
    
    return FoundActors;
}

bool UBuild_IntegrationValidator::ValidateClassLoading(const FString& ClassName)
{
    FString FullClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *FullClassName);
    return LoadedClass != nullptr;
}