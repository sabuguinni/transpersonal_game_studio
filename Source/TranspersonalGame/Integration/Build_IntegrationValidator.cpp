#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Light.h"
#include "Components/LightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Kismet/GameplayStatics.h"

ABuild_IntegrationValidator::ABuild_IntegrationValidator()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize validation data
    TotalActorCount = 0;
    DinosaurCount = 0;
    bPerformanceWithinLimits = false;
    LastValidationTime = FDateTime::Now();
}

void ABuild_IntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-run validation on begin play
    RunFullValidation();
}

void ABuild_IntegrationValidator::RunFullValidation()
{
    ClearValidationResults();
    
    UE_LOG(LogTemp, Warning, TEXT("Build Integration Validator: Starting full validation"));
    
    ValidateModuleCompilation();
    ValidateActorCounts();
    ValidateSystemIntegration();
    ValidatePerformance();
    
    LastValidationTime = FDateTime::Now();
    
    GenerateValidationReport();
}

void ABuild_IntegrationValidator::ValidateModuleCompilation()
{
    // Test core module classes
    TArray<TPair<FString, FString>> TestClasses = {
        {TEXT("TranspersonalGameState"), TEXT("/Script/TranspersonalGame.TranspersonalGameState")},
        {TEXT("TranspersonalCharacter"), TEXT("/Script/TranspersonalGame.TranspersonalCharacter")},
        {TEXT("PCGWorldGenerator"), TEXT("/Script/TranspersonalGame.PCGWorldGenerator")},
        {TEXT("FoliageManager"), TEXT("/Script/TranspersonalGame.FoliageManager")},
        {TEXT("CrowdSimulationManager"), TEXT("/Script/TranspersonalGame.CrowdSimulationManager")},
        {TEXT("ProceduralWorldManager"), TEXT("/Script/TranspersonalGame.ProceduralWorldManager")},
        {TEXT("BuildIntegrationManager"), TEXT("/Script/TranspersonalGame.BuildIntegrationManager")}
    };

    int32 LoadedClasses = 0;
    for (const auto& TestClass : TestClasses)
    {
        if (ValidateClassLoading(TestClass.Key, TestClass.Value))
        {
            LoadedClasses++;
        }
    }

    EBuild_ValidationStatus Status = (LoadedClasses == TestClasses.Num()) ? 
        EBuild_ValidationStatus::Pass : EBuild_ValidationStatus::Warning;
    
    FString Message = FString::Printf(TEXT("Loaded %d/%d core classes"), LoadedClasses, TestClasses.Num());
    AddValidationResult(TEXT("Module Compilation"), Status, Message, LoadedClasses);
}

void ABuild_IntegrationValidator::ValidateActorCounts()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationResult(TEXT("Actor Count"), EBuild_ValidationStatus::Fail, TEXT("No world found"));
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    TotalActorCount = AllActors.Num();

    // Count dinosaurs (actors with dinosaur-related labels)
    TArray<FString> DinosaurLabels = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), 
                                     TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")};
    
    DinosaurCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            FString ActorLabel = Actor->GetActorLabel().ToLower();
            for (const FString& DinoLabel : DinosaurLabels)
            {
                if (ActorLabel.Contains(DinoLabel))
                {
                    DinosaurCount++;
                    break;
                }
            }
        }
    }

    // Validate counts against limits
    EBuild_ValidationStatus Status = EBuild_ValidationStatus::Pass;
    FString Message = FString::Printf(TEXT("Total: %d, Dinosaurs: %d"), TotalActorCount, DinosaurCount);
    
    if (TotalActorCount > 8000)
    {
        Status = EBuild_ValidationStatus::Warning;
        Message += TEXT(" - Total actor count high");
    }
    
    if (DinosaurCount > 150)
    {
        Status = EBuild_ValidationStatus::Warning;
        Message += TEXT(" - Dinosaur count high");
    }

    AddValidationResult(TEXT("Actor Counts"), Status, Message, TotalActorCount);
}

void ABuild_IntegrationValidator::ValidateSystemIntegration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationResult(TEXT("System Integration"), EBuild_ValidationStatus::Fail, TEXT("No world found"));
        return;
    }

    // Check for essential systems
    int32 SystemsFound = 0;
    
    // Check for lighting
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0) SystemsFound++;

    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    if (SkyLights.Num() > 0) SystemsFound++;

    // Check for static meshes (environment)
    TArray<AActor*> StaticMeshes;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshes);
    if (StaticMeshes.Num() > 0) SystemsFound++;

    EBuild_ValidationStatus Status = (SystemsFound >= 3) ? 
        EBuild_ValidationStatus::Pass : EBuild_ValidationStatus::Warning;
    
    FString Message = FString::Printf(TEXT("Found %d essential systems"), SystemsFound);
    AddValidationResult(TEXT("System Integration"), Status, Message, SystemsFound);
}

void ABuild_IntegrationValidator::ValidatePerformance()
{
    // Performance validation based on actor counts and system complexity
    bPerformanceWithinLimits = (TotalActorCount <= 8000 && DinosaurCount <= 150);
    
    EBuild_ValidationStatus Status = bPerformanceWithinLimits ? 
        EBuild_ValidationStatus::Pass : EBuild_ValidationStatus::Warning;
    
    FString Message = bPerformanceWithinLimits ? 
        TEXT("Performance within acceptable limits") : 
        TEXT("Performance may be impacted by high actor counts");
    
    AddValidationResult(TEXT("Performance"), Status, Message);
}

void ABuild_IntegrationValidator::GenerateValidationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION VALIDATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Validation Time: %s"), *LastValidationTime.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Total Systems Tested: %d"), SystemValidations.Num());
    
    for (const FBuild_SystemValidation& Validation : SystemValidations)
    {
        FString StatusString;
        switch (Validation.Status)
        {
            case EBuild_ValidationStatus::Pass: StatusString = TEXT("PASS"); break;
            case EBuild_ValidationStatus::Fail: StatusString = TEXT("FAIL"); break;
            case EBuild_ValidationStatus::Warning: StatusString = TEXT("WARN"); break;
            default: StatusString = TEXT("UNKNOWN"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("%s: %s - %s"), 
               *Validation.SystemName, *StatusString, *Validation.ErrorMessage);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Overall Status: %s"), 
           *UEnum::GetValueAsString(GetOverallStatus()));
    UE_LOG(LogTemp, Warning, TEXT("=== END VALIDATION REPORT ==="));
}

EBuild_ValidationStatus ABuild_IntegrationValidator::GetOverallStatus() const
{
    bool bHasFail = false;
    bool bHasWarning = false;
    
    for (const FBuild_SystemValidation& Validation : SystemValidations)
    {
        if (Validation.Status == EBuild_ValidationStatus::Fail)
        {
            bHasFail = true;
        }
        else if (Validation.Status == EBuild_ValidationStatus::Warning)
        {
            bHasWarning = true;
        }
    }
    
    if (bHasFail) return EBuild_ValidationStatus::Fail;
    if (bHasWarning) return EBuild_ValidationStatus::Warning;
    return EBuild_ValidationStatus::Pass;
}

FString ABuild_IntegrationValidator::GetValidationSummary() const
{
    int32 PassCount = 0;
    int32 FailCount = 0;
    int32 WarnCount = 0;
    
    for (const FBuild_SystemValidation& Validation : SystemValidations)
    {
        switch (Validation.Status)
        {
            case EBuild_ValidationStatus::Pass: PassCount++; break;
            case EBuild_ValidationStatus::Fail: FailCount++; break;
            case EBuild_ValidationStatus::Warning: WarnCount++; break;
        }
    }
    
    return FString::Printf(TEXT("Pass: %d, Warn: %d, Fail: %d"), PassCount, WarnCount, FailCount);
}

void ABuild_IntegrationValidator::AddValidationResult(const FString& SystemName, EBuild_ValidationStatus Status, const FString& ErrorMessage, int32 ActorCount)
{
    FBuild_SystemValidation NewValidation;
    NewValidation.SystemName = SystemName;
    NewValidation.Status = Status;
    NewValidation.ErrorMessage = ErrorMessage;
    NewValidation.ActorCount = ActorCount;
    
    SystemValidations.Add(NewValidation);
}

void ABuild_IntegrationValidator::ClearValidationResults()
{
    SystemValidations.Empty();
}

bool ABuild_IntegrationValidator::ValidateClassLoading(const FString& ClassName, const FString& ClassPath)
{
    UClass* LoadedClass = StaticLoadClass(UObject::StaticClass(), nullptr, *ClassPath);
    if (LoadedClass)
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully loaded class: %s"), *ClassName);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load class: %s at path: %s"), *ClassName, *ClassPath);
        return false;
    }
}