#include "BuildValidationSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "BuildValidationActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

UBuildValidationSubsystem::UBuildValidationSubsystem()
{
    bIsValidationEnabled = true;
    ValidationFrequency = 10.0f;
    LastValidationTime = 0.0f;
    OverallValidationScore = 0.0f;
    ActiveValidationActors.Empty();
}

void UBuildValidationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationSubsystem: Initializing validation subsystem"));
    
    // Initialize validation state
    bIsValidationEnabled = true;
    ValidationFrequency = 10.0f;
    LastValidationTime = 0.0f;
    OverallValidationScore = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSubsystem: Initialization complete"));
}

void UBuildValidationSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationSubsystem: Deinitializing validation subsystem"));
    
    // Clear validation actors
    ActiveValidationActors.Empty();
    
    Super::Deinitialize();
}

bool UBuildValidationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UBuildValidationSubsystem::Tick(float DeltaTime)
{
    if (!bIsValidationEnabled)
        return;
    
    LastValidationTime += DeltaTime;
    
    // Perform periodic validation
    if (LastValidationTime >= ValidationFrequency)
    {
        PerformSystemValidation();
        LastValidationTime = 0.0f;
    }
}

bool UBuildValidationSubsystem::IsTickable() const
{
    return bIsValidationEnabled;
}

TStatId UBuildValidationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UBuildValidationSubsystem, STATGROUP_Tickables);
}

void UBuildValidationSubsystem::PerformSystemValidation()
{
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSubsystem: Performing system-wide validation"));
    
    float TotalScore = 0.0f;
    int32 ValidActors = 0;
    
    // Validate all registered validation actors
    for (TWeakObjectPtr<ABuildValidationActor> ActorPtr : ActiveValidationActors)
    {
        if (ActorPtr.IsValid())
        {
            ABuildValidationActor* Actor = ActorPtr.Get();
            Actor->ForceValidation();
            TotalScore += Actor->GetValidationScore();
            ValidActors++;
        }
    }
    
    // Calculate overall score
    if (ValidActors > 0)
    {
        OverallValidationScore = TotalScore / ValidActors;
    }
    else
    {
        OverallValidationScore = 0.0f;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationSubsystem: System validation complete - Score: %.1f%% (%d actors)"), 
           OverallValidationScore, ValidActors);
    
    // Broadcast validation result
    OnSystemValidationComplete.Broadcast(OverallValidationScore >= 75.0f, OverallValidationScore);
}

void UBuildValidationSubsystem::RegisterValidationActor(ABuildValidationActor* Actor)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildValidationSubsystem: Cannot register null validation actor"));
        return;
    }
    
    // Add to active actors list
    ActiveValidationActors.AddUnique(Actor);
    
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSubsystem: Registered validation actor '%s' (Total: %d)"), 
           *Actor->GetName(), ActiveValidationActors.Num());
}

void UBuildValidationSubsystem::UnregisterValidationActor(ABuildValidationActor* Actor)
{
    if (!Actor)
        return;
    
    // Remove from active actors list
    ActiveValidationActors.RemoveAll([Actor](const TWeakObjectPtr<ABuildValidationActor>& Ptr) {
        return Ptr.Get() == Actor;
    });
    
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSubsystem: Unregistered validation actor '%s' (Remaining: %d)"), 
           *Actor->GetName(), ActiveValidationActors.Num());
}

void UBuildValidationSubsystem::SetValidationEnabled(bool bEnabled)
{
    bIsValidationEnabled = bEnabled;
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationSubsystem: Validation %s"), 
           bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UBuildValidationSubsystem::SetValidationFrequency(float Frequency)
{
    ValidationFrequency = FMath::Max(1.0f, Frequency);
    UE_LOG(LogTemp, Log, TEXT("BuildValidationSubsystem: Validation frequency set to %.1f seconds"), ValidationFrequency);
}

float UBuildValidationSubsystem::GetOverallValidationScore() const
{
    return OverallValidationScore;
}

bool UBuildValidationSubsystem::IsSystemValidationPassing() const
{
    return OverallValidationScore >= 75.0f;
}

int32 UBuildValidationSubsystem::GetActiveValidationActorCount() const
{
    // Clean up invalid weak pointers and return count
    int32 ValidCount = 0;
    for (const TWeakObjectPtr<ABuildValidationActor>& ActorPtr : ActiveValidationActors)
    {
        if (ActorPtr.IsValid())
        {
            ValidCount++;
        }
    }
    return ValidCount;
}

void UBuildValidationSubsystem::ForceSystemValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationSubsystem: Force system validation requested"));
    PerformSystemValidation();
}

TArray<ABuildValidationActor*> UBuildValidationSubsystem::GetActiveValidationActors() const
{
    TArray<ABuildValidationActor*> ValidActors;
    
    for (const TWeakObjectPtr<ABuildValidationActor>& ActorPtr : ActiveValidationActors)
    {
        if (ActorPtr.IsValid())
        {
            ValidActors.Add(ActorPtr.Get());
        }
    }
    
    return ValidActors;
}