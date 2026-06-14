#include "BuildIntegrationValidator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "Engine/Light.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UBuildIntegrationValidator::UBuildIntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    MaxDinosaurCount = 150;
    MaxTotalActorCount = 8000;
    MinPerformanceThreshold = 30.0f;
    
    InitializeEssentialSystems();
}

void UBuildIntegrationValidator::InitializeEssentialSystems()
{
    EssentialSystems.Empty();
    EssentialSystems.Add(TEXT("GameMode"), TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
    EssentialSystems.Add(TEXT("Character"), TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    EssentialSystems.Add(TEXT("GameState"), TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    EssentialSystems.Add(TEXT("WorldGen"), TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    EssentialSystems.Add(TEXT("Foliage"), TEXT("/Script/TranspersonalGame.FoliageManager"));
    EssentialSystems.Add(TEXT("Crowd"), TEXT("/Script/TranspersonalGame.CrowdSimulationManager"));
    EssentialSystems.Add(TEXT("BuildMgr"), TEXT("/Script/TranspersonalGame.BuildIntegrationManager"));
}

FBuild_IntegrationReport UBuildIntegrationValidator::ValidateFullBuild()
{
    FBuild_IntegrationReport Report;
    Report.ValidationTimestamp = FDateTime::Now();
    
    // Validate all essential systems
    for (const auto& SystemPair : EssentialSystems)
    {
        FBuild_SystemStatus Status;
        Status.SystemName = SystemPair.Key;
        Status.bIsLoaded = ValidateClassLoading(SystemPair.Value);
        Status.bIsCompiled = Status.bIsLoaded; // If loaded, it's compiled
        Status.ValidationResult = Status.bIsLoaded ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Error;
        Status.ErrorMessage = Status.bIsLoaded ? TEXT("") : FString::Printf(TEXT("Failed to load class: %s"), *SystemPair.Value);
        
        Report.SystemStatuses.Add(Status);
        
        LogValidationResult(Status.SystemName, Status.ValidationResult, Status.ErrorMessage);
    }
    
    // Count actors
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        Report.TotalActorCount = AllActors.Num();
        
        // Count dinosaurs (actors with dinosaur-related labels)
        TArray<FString> DinosaurLabels = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), 
                                         TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")};
        
        Report.DinosaurCount = 0;
        for (AActor* Actor : AllActors)
        {
            FString ActorLabel = Actor->GetActorLabel().ToLower();
            for (const FString& DinoLabel : DinosaurLabels)
            {
                if (ActorLabel.Contains(DinoLabel))
                {
                    Report.DinosaurCount++;
                    break;
                }
            }
        }
    }
    
    // Calculate performance score
    Report.PerformanceScore = CalculatePerformanceScore();
    
    // Determine overall build health
    int32 SuccessfulSystems = 0;
    for (const FBuild_SystemStatus& Status : Report.SystemStatuses)
    {
        if (Status.ValidationResult == EBuild_ValidationResult::Success)
        {
            SuccessfulSystems++;
        }
    }
    
    float SystemSuccessRate = (float)SuccessfulSystems / (float)Report.SystemStatuses.Num();
    bool bActorCountsHealthy = (Report.DinosaurCount <= MaxDinosaurCount) && (Report.TotalActorCount <= MaxTotalActorCount);
    bool bPerformanceHealthy = Report.PerformanceScore >= MinPerformanceThreshold;
    
    Report.bBuildHealthy = (SystemSuccessRate >= 0.8f) && bActorCountsHealthy && bPerformanceHealthy;
    
    UE_LOG(LogTemp, Warning, TEXT("Build Validation Complete - Healthy: %s, Systems: %d/%d, Actors: %d/%d, Dinos: %d/%d, Performance: %.1f"),
           Report.bBuildHealthy ? TEXT("YES") : TEXT("NO"),
           SuccessfulSystems, Report.SystemStatuses.Num(),
           Report.TotalActorCount, MaxTotalActorCount,
           Report.DinosaurCount, MaxDinosaurCount,
           Report.PerformanceScore);
    
    return Report;
}

bool UBuildIntegrationValidator::ValidateSystemIntegration(const FString& SystemName, const FString& ClassPath)
{
    bool bIsValid = ValidateClassLoading(ClassPath);
    EBuild_ValidationResult Result = bIsValid ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Error;
    FString Message = bIsValid ? TEXT("System integration successful") : FString::Printf(TEXT("Failed to validate system: %s"), *ClassPath);
    
    LogValidationResult(SystemName, Result, Message);
    
    return bIsValid;
}

void UBuildIntegrationValidator::EnforceActorLimits()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    // Identify dinosaurs and essential actors
    TArray<FString> DinosaurLabels = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), 
                                     TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")};
    TArray<FString> EssentialLabels = {TEXT("playerstart"), TEXT("directionallight"), TEXT("skylight"), 
                                      TEXT("skyatmosphere"), TEXT("fog")};
    
    TArray<AActor*> Dinosaurs;
    TArray<AActor*> EssentialActors;
    TArray<AActor*> Props;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        bool bIsDinosaur = false;
        bool bIsEssential = false;
        
        for (const FString& DinoLabel : DinosaurLabels)
        {
            if (ActorLabel.Contains(DinoLabel))
            {
                Dinosaurs.Add(Actor);
                bIsDinosaur = true;
                break;
            }
        }
        
        if (!bIsDinosaur)
        {
            for (const FString& EssentialLabel : EssentialLabels)
            {
                if (ActorLabel.Contains(EssentialLabel))
                {
                    EssentialActors.Add(Actor);
                    bIsEssential = true;
                    break;
                }
            }
        }
        
        if (!bIsDinosaur && !bIsEssential)
        {
            Props.Add(Actor);
        }
    }
    
    // Enforce dinosaur limit
    if (Dinosaurs.Num() > MaxDinosaurCount)
    {
        int32 ToRemove = Dinosaurs.Num() - MaxDinosaurCount;
        for (int32 i = 0; i < ToRemove && i < Dinosaurs.Num(); i++)
        {
            if (Dinosaurs[i] && IsValid(Dinosaurs[i]))
            {
                Dinosaurs[i]->Destroy();
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("Enforced dinosaur limit: removed %d dinosaurs"), ToRemove);
    }
    
    // Enforce total actor limit
    int32 CurrentTotal = EssentialActors.Num() + FMath::Min(Dinosaurs.Num(), MaxDinosaurCount) + Props.Num();
    if (CurrentTotal > MaxTotalActorCount)
    {
        int32 PropsToRemove = CurrentTotal - MaxTotalActorCount;
        for (int32 i = 0; i < PropsToRemove && i < Props.Num(); i++)
        {
            if (Props[i] && IsValid(Props[i]))
            {
                Props[i]->Destroy();
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("Enforced total actor limit: removed %d props"), PropsToRemove);
    }
}

float UBuildIntegrationValidator::CalculatePerformanceScore()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    // Simple performance scoring based on actor counts and types
    float Score = 100.0f;
    
    // Penalty for too many actors
    if (AllActors.Num() > MaxTotalActorCount * 0.8f)
    {
        Score -= 20.0f;
    }
    
    // Count performance-heavy actors
    int32 StaticMeshCount = 0;
    int32 LightCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        
        if (Actor->FindComponentByClass<UStaticMeshComponent>())
        {
            StaticMeshCount++;
        }
        
        if (Actor->IsA<ALight>())
        {
            LightCount++;
        }
    }
    
    // Apply penalties for performance-heavy elements
    if (StaticMeshCount > 5000) Score -= 15.0f;
    if (LightCount > 50) Score -= 10.0f;
    
    return FMath::Clamp(Score, 0.0f, 100.0f);
}

TArray<FBuild_SystemStatus> UBuildIntegrationValidator::GetCriticalSystemStatuses()
{
    TArray<FBuild_SystemStatus> CriticalStatuses;
    
    for (const auto& SystemPair : EssentialSystems)
    {
        FBuild_SystemStatus Status;
        Status.SystemName = SystemPair.Key;
        Status.bIsLoaded = ValidateClassLoading(SystemPair.Value);
        Status.bIsCompiled = Status.bIsLoaded;
        Status.ValidationResult = Status.bIsLoaded ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Critical;
        Status.ErrorMessage = Status.bIsLoaded ? TEXT("") : FString::Printf(TEXT("CRITICAL: Failed to load %s"), *SystemPair.Value);
        
        if (Status.ValidationResult == EBuild_ValidationResult::Critical)
        {
            CriticalStatuses.Add(Status);
        }
    }
    
    return CriticalStatuses;
}

bool UBuildIntegrationValidator::ValidateClassLoading(const FString& ClassPath)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    return LoadedClass != nullptr;
}

void UBuildIntegrationValidator::LogValidationResult(const FString& SystemName, EBuild_ValidationResult Result, const FString& Message)
{
    FString ResultString;
    switch (Result)
    {
        case EBuild_ValidationResult::Success:
            ResultString = TEXT("SUCCESS");
            UE_LOG(LogTemp, Log, TEXT("Build Validation [%s]: %s - %s"), *SystemName, *ResultString, *Message);
            break;
        case EBuild_ValidationResult::Warning:
            ResultString = TEXT("WARNING");
            UE_LOG(LogTemp, Warning, TEXT("Build Validation [%s]: %s - %s"), *SystemName, *ResultString, *Message);
            break;
        case EBuild_ValidationResult::Error:
            ResultString = TEXT("ERROR");
            UE_LOG(LogTemp, Error, TEXT("Build Validation [%s]: %s - %s"), *SystemName, *ResultString, *Message);
            break;
        case EBuild_ValidationResult::Critical:
            ResultString = TEXT("CRITICAL");
            UE_LOG(LogTemp, Error, TEXT("Build Validation [%s]: %s - %s"), *SystemName, *ResultString, *Message);
            break;
    }
}