#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Atmosphere/AtmosphericFog.h"
#include "SharedTypes.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Check every 5 seconds
    
    bAutoActivate = true;
    bWantsInitializeComponent = true;
    
    // Initialize validation settings
    MaxAllowedDuplicates = 1;
    bPerformAutomaticCleanup = true;
    bLogValidationResults = true;
    
    // Critical actor types that should not be duplicated
    CriticalActorTypes.Add(TEXT("DirectionalLight"));
    CriticalActorTypes.Add(TEXT("SkyLight"));
    CriticalActorTypes.Add(TEXT("ExponentialHeightFog"));
    CriticalActorTypes.Add(TEXT("SkyAtmosphere"));
    CriticalActorTypes.Add(TEXT("AtmosphericFog"));
}

void UBuildIntegrationManager::InitializeComponent()
{
    Super::InitializeComponent();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Component initialized"));
    
    // Perform initial validation
    if (GetWorld())
    {
        ValidateMapIntegrity();
    }
}

void UBuildIntegrationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic validation
    if (bPerformAutomaticCleanup)
    {
        ValidateMapIntegrity();
    }
}

void UBuildIntegrationManager::ValidateMapIntegrity()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: No valid world found"));
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    // Count actors by type
    TMap<FString, TArray<AActor*>> ActorsByType;
    
    for (AActor* Actor : AllActors)
    {
        if (IsValid(Actor))
        {
            FString ActorClassName = Actor->GetClass()->GetName();
            ActorsByType.FindOrAdd(ActorClassName).Add(Actor);
        }
    }
    
    // Check for duplicates in critical types
    TArray<FString> ProblemsFound;
    
    for (const FString& CriticalType : CriticalActorTypes)
    {
        if (ActorsByType.Contains(CriticalType))
        {
            TArray<AActor*>& ActorsOfType = ActorsByType[CriticalType];
            
            if (ActorsOfType.Num() > MaxAllowedDuplicates)
            {
                FString Problem = FString::Printf(TEXT("Found %d %s actors (max allowed: %d)"), 
                    ActorsOfType.Num(), *CriticalType, MaxAllowedDuplicates);
                ProblemsFound.Add(Problem);
                
                if (bLogValidationResults)
                {
                    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: %s"), *Problem);
                }
                
                // Auto-cleanup if enabled
                if (bPerformAutomaticCleanup)
                {
                    CleanupDuplicateActors(CriticalType, ActorsOfType);
                }
            }
        }
    }
    
    // Log validation results
    if (bLogValidationResults)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Validation complete. Total actors: %d"), AllActors.Num());
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Problems found: %d"), ProblemsFound.Num());
        
        for (const FString& Problem : ProblemsFound)
        {
            UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: ISSUE - %s"), *Problem);
        }
    }
    
    // Update validation status
    LastValidationTime = GetWorld()->GetTimeSeconds();
    ValidationProblems = ProblemsFound;
}

void UBuildIntegrationManager::CleanupDuplicateActors(const FString& ActorType, TArray<AActor*>& ActorsOfType)
{
    if (ActorsOfType.Num() <= MaxAllowedDuplicates)
    {
        return; // No cleanup needed
    }
    
    // Sort actors by creation time (keep the oldest)
    ActorsOfType.Sort([](const AActor& A, const AActor& B) {
        return A.GetUniqueID() < B.GetUniqueID();
    });
    
    // Remove excess actors
    int32 ActorsToRemove = ActorsOfType.Num() - MaxAllowedDuplicates;
    
    for (int32 i = MaxAllowedDuplicates; i < ActorsOfType.Num(); ++i)
    {
        AActor* ActorToRemove = ActorsOfType[i];
        
        if (IsValid(ActorToRemove))
        {
            UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Removing duplicate %s: %s"), 
                *ActorType, *ActorToRemove->GetName());
            
            ActorToRemove->Destroy();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Cleaned up %d duplicate %s actors"), 
        ActorsToRemove, *ActorType);
}

void UBuildIntegrationManager::ForceValidation()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Force validation requested"));
    ValidateMapIntegrity();
}

void UBuildIntegrationManager::SetAutomaticCleanup(bool bEnabled)
{
    bPerformAutomaticCleanup = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Automatic cleanup %s"), 
        bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

TArray<FString> UBuildIntegrationManager::GetValidationProblems() const
{
    return ValidationProblems;
}

float UBuildIntegrationManager::GetLastValidationTime() const
{
    return LastValidationTime;
}

bool UBuildIntegrationManager::IsMapValid() const
{
    return ValidationProblems.Num() == 0;
}

void UBuildIntegrationManager::AddCriticalActorType(const FString& ActorType)
{
    if (!CriticalActorTypes.Contains(ActorType))
    {
        CriticalActorTypes.Add(ActorType);
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Added critical actor type: %s"), *ActorType);
    }
}

void UBuildIntegrationManager::RemoveCriticalActorType(const FString& ActorType)
{
    if (CriticalActorTypes.Contains(ActorType))
    {
        CriticalActorTypes.Remove(ActorType);
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Removed critical actor type: %s"), *ActorType);
    }
}

int32 UBuildIntegrationManager::GetActorCount(const FString& ActorType) const
{
    if (!GetWorld())
    {
        return 0;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 Count = 0;
    for (AActor* Actor : AllActors)
    {
        if (IsValid(Actor) && Actor->GetClass()->GetName() == ActorType)
        {
            Count++;
        }
    }
    
    return Count;
}