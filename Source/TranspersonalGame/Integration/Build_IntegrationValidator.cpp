#include "Build_IntegrationValidator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    MaxTotalActors = 8000;
    MaxDinosaurActors = 150;
    
    CoreSystemClasses = {
        TEXT("TranspersonalGameState"),
        TEXT("TranspersonalCharacter"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("ProceduralWorldManager"),
        TEXT("BuildIntegrationManager")
    };
    
    EssentialActorTypes = {
        TEXT("PlayerStart"),
        TEXT("DirectionalLight"),
        TEXT("SkyLight"),
        TEXT("SkyAtmosphere"),
        TEXT("ExponentialHeightFog")
    };
}

FBuild_IntegrationReport UBuild_IntegrationValidator::ValidateAllSystems()
{
    FBuild_IntegrationReport Report;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_IntegrationValidator: No valid world found"));
        return Report;
    }
    
    // Count total actors
    Report.TotalActors = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        Report.TotalActors++;
    }
    
    // Validate each core system
    for (const FString& SystemClass : CoreSystemClasses)
    {
        FBuild_SystemReport SystemReport = ValidateSystem(SystemClass, SystemClass);
        Report.SystemReports.Add(SystemReport);
        
        if (SystemReport.Status == EBuild_SystemStatus::Functional)
        {
            Report.ReadySystems++;
        }
    }
    
    // Calculate integration score
    Report.IntegrationScore = CalculateIntegrationScore();
    Report.bBuildReady = Report.IntegrationScore >= 70.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Integration Report: %d/%d systems ready, Score: %.1f, Total Actors: %d"), 
           Report.ReadySystems, CoreSystemClasses.Num(), Report.IntegrationScore, Report.TotalActors);
    
    return Report;
}

bool UBuild_IntegrationValidator::ValidateSystemClasses()
{
    int32 LoadedClasses = 0;
    
    for (const FString& ClassName : CoreSystemClasses)
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (LoadedClass)
        {
            LoadedClasses++;
            UE_LOG(LogTemp, Log, TEXT("Class loaded successfully: %s"), *ClassName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load class: %s"), *ClassName);
        }
    }
    
    return LoadedClasses >= (CoreSystemClasses.Num() * 0.7f); // 70% threshold
}

bool UBuild_IntegrationValidator::ValidateActorCounts()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    int32 TotalActors = 0;
    int32 DinosaurActors = 0;
    
    TArray<FString> DinosaurLabels = {
        TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"),
        TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")
    };
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        TotalActors++;
        
        FString ActorLabel = ActorItr->GetActorLabel().ToLower();
        for (const FString& DinoLabel : DinosaurLabels)
        {
            if (ActorLabel.Contains(DinoLabel))
            {
                DinosaurActors++;
                break;
            }
        }
    }
    
    bool bCountsValid = (TotalActors <= MaxTotalActors) && (DinosaurActors <= MaxDinosaurActors);
    
    UE_LOG(LogTemp, Warning, TEXT("Actor Validation: Total=%d (max %d), Dinosaurs=%d (max %d), Valid=%s"),
           TotalActors, MaxTotalActors, DinosaurActors, MaxDinosaurActors, 
           bCountsValid ? TEXT("YES") : TEXT("NO"));
    
    return bCountsValid;
}

bool UBuild_IntegrationValidator::ValidateCrossSystemIntegration()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Check for essential actors
    int32 EssentialFound = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        FString ActorClass = ActorItr->GetClass()->GetName();
        
        for (const FString& EssentialType : EssentialActorTypes)
        {
            if (ActorClass.Contains(EssentialType))
            {
                EssentialFound++;
                break;
            }
        }
    }
    
    bool bIntegrationValid = EssentialFound >= (EssentialActorTypes.Num() * 0.8f); // 80% threshold
    
    UE_LOG(LogTemp, Warning, TEXT("Cross-System Integration: %d/%d essential actors found, Valid=%s"),
           EssentialFound, EssentialActorTypes.Num(), bIntegrationValid ? TEXT("YES") : TEXT("NO"));
    
    return bIntegrationValid;
}

float UBuild_IntegrationValidator::CalculateIntegrationScore()
{
    float Score = 0.0f;
    
    // System class validation (40 points)
    if (ValidateSystemClasses())
    {
        Score += 40.0f;
    }
    
    // Actor count validation (30 points)
    if (ValidateActorCounts())
    {
        Score += 30.0f;
    }
    
    // Cross-system integration (30 points)
    if (ValidateCrossSystemIntegration())
    {
        Score += 30.0f;
    }
    
    return FMath::Clamp(Score, 0.0f, 100.0f);
}

void UBuild_IntegrationValidator::EnforceActorCaps()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> AllActors;
    TArray<AActor*> DinosaurActors;
    TArray<AActor*> EssentialActors;
    
    TArray<FString> DinosaurLabels = {
        TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"),
        TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")
    };
    
    // Categorize actors
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        AllActors.Add(Actor);
        
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        FString ActorClass = Actor->GetClass()->GetName().ToLower();
        
        // Check if dinosaur
        bool bIsDinosaur = false;
        for (const FString& DinoLabel : DinosaurLabels)
        {
            if (ActorLabel.Contains(DinoLabel))
            {
                DinosaurActors.Add(Actor);
                bIsDinosaur = true;
                break;
            }
        }
        
        // Check if essential
        if (!bIsDinosaur)
        {
            for (const FString& EssentialType : EssentialActorTypes)
            {
                if (ActorClass.Contains(EssentialType.ToLower()))
                {
                    EssentialActors.Add(Actor);
                    break;
                }
            }
        }
    }
    
    // Enforce dinosaur cap
    if (DinosaurActors.Num() > MaxDinosaurActors)
    {
        DinosaurActors.Sort([](const AActor& A, const AActor& B) {
            return FMath::RandBool();
        });
        
        for (int32 i = MaxDinosaurActors; i < DinosaurActors.Num(); i++)
        {
            if (DinosaurActors[i] && IsValid(DinosaurActors[i]))
            {
                DinosaurActors[i]->Destroy();
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Enforced dinosaur cap: removed %d excess dinosaurs"), 
               DinosaurActors.Num() - MaxDinosaurActors);
    }
    
    // Enforce total actor cap
    if (AllActors.Num() > MaxTotalActors)
    {
        TArray<AActor*> NonEssentialActors;
        for (AActor* Actor : AllActors)
        {
            if (!EssentialActors.Contains(Actor))
            {
                NonEssentialActors.Add(Actor);
            }
        }
        
        NonEssentialActors.Sort([](const AActor& A, const AActor& B) {
            return FMath::RandBool();
        });
        
        int32 ActorsToRemove = AllActors.Num() - MaxTotalActors;
        for (int32 i = 0; i < FMath::Min(ActorsToRemove, NonEssentialActors.Num()); i++)
        {
            if (NonEssentialActors[i] && IsValid(NonEssentialActors[i]))
            {
                NonEssentialActors[i]->Destroy();
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Enforced total actor cap: removed %d excess actors"), ActorsToRemove);
    }
}

FBuild_SystemReport UBuild_IntegrationValidator::ValidateSystem(const FString& SystemName, const FString& ClassName)
{
    FBuild_SystemReport Report;
    Report.SystemName = SystemName;
    
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    if (!SystemClass)
    {
        Report.Status = EBuild_SystemStatus::NotLoaded;
        Report.ErrorMessage = TEXT("Class not found");
        return Report;
    }
    
    Report.Status = EBuild_SystemStatus::Loaded;
    
    // Count actors of this type
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (ActorItr->GetClass() == SystemClass)
            {
                Report.ActorCount++;
            }
        }
        
        if (Report.ActorCount > 0)
        {
            Report.Status = EBuild_SystemStatus::Functional;
        }
    }
    
    return Report;
}

int32 UBuild_IntegrationValidator::CountActorsByType(const FString& ActorType)
{
    UWorld* World = GetWorld();
    if (!World) return 0;
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        if (ActorItr->GetClass()->GetName().Contains(ActorType))
        {
            Count++;
        }
    }
    
    return Count;
}