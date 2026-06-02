#include "Core_PhysicsValidation.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Character.h"

DEFINE_LOG_CATEGORY(LogPhysicsValidation);

UCore_PhysicsValidation::UCore_PhysicsValidation()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Validate every second
    
    // Initialize validation thresholds
    MaxVelocityThreshold = 5000.0f; // cm/s
    MaxAngularVelocityThreshold = 720.0f; // degrees/s
    MaxPhysicsActorsThreshold = 500;
    MinFrameRateThreshold = 30.0f;
    
    ValidationResults.Reset();
    LastValidationTime = 0.0f;
    bIsValidationEnabled = true;
}

void UCore_PhysicsValidation::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPhysicsValidation, Log, TEXT("Physics Validation System initialized"));
    
    // Register for physics events
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(ValidationTimerHandle, this, 
            &UCore_PhysicsValidation::PerformValidationCheck, 1.0f, true);
    }
}

void UCore_PhysicsValidation::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    Super::EndPlay(EndPlayReason);
}

void UCore_PhysicsValidation::TickComponent(float DeltaTime, ELevelTick TickType, 
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsValidationEnabled)
    {
        return;
    }
    
    // Update frame rate tracking
    CurrentFrameRate = 1.0f / DeltaTime;
    
    // Check for critical physics issues
    if (CurrentFrameRate < MinFrameRateThreshold)
    {
        UE_LOG(LogPhysicsValidation, Warning, TEXT("Low frame rate detected: %.2f FPS"), CurrentFrameRate);
    }
}

void UCore_PhysicsValidation::PerformValidationCheck()
{
    if (!bIsValidationEnabled)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FCore_PhysicsValidationResult Result;
    Result.Timestamp = FDateTime::Now();
    Result.bIsValid = true;
    Result.ValidationMessages.Empty();
    
    // Validate physics actors count
    int32 PhysicsActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && HasPhysicsComponents(Actor))
        {
            PhysicsActorCount++;
        }
    }
    
    if (PhysicsActorCount > MaxPhysicsActorsThreshold)
    {
        Result.bIsValid = false;
        Result.ValidationMessages.Add(FString::Printf(TEXT("Too many physics actors: %d (max: %d)"), 
            PhysicsActorCount, MaxPhysicsActorsThreshold));
    }
    
    // Validate velocity constraints
    ValidateVelocityConstraints(Result);
    
    // Validate collision settings
    ValidateCollisionSettings(Result);
    
    // Validate physics materials
    ValidatePhysicsMaterials(Result);
    
    // Store result
    ValidationResults.Add(Result);
    
    // Keep only last 100 results
    if (ValidationResults.Num() > 100)
    {
        ValidationResults.RemoveAt(0);
    }
    
    LastValidationTime = World->GetTimeSeconds();
    
    // Log critical issues
    if (!Result.bIsValid)
    {
        for (const FString& Message : Result.ValidationMessages)
        {
            UE_LOG(LogPhysicsValidation, Error, TEXT("Physics Validation Failed: %s"), *Message);
        }
    }
}

bool UCore_PhysicsValidation::HasPhysicsComponents(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    // Check for static mesh components with physics
    TArray<UStaticMeshComponent*> StaticMeshComponents;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
    
    for (UStaticMeshComponent* Component : StaticMeshComponents)
    {
        if (Component && Component->GetBodyInstance() && Component->GetBodyInstance()->bSimulatePhysics)
        {
            return true;
        }
    }
    
    // Check for skeletal mesh components with physics
    TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
    
    for (USkeletalMeshComponent* Component : SkeletalMeshComponents)
    {
        if (Component && Component->GetBodyInstance() && Component->GetBodyInstance()->bSimulatePhysics)
        {
            return true;
        }
    }
    
    return false;
}

void UCore_PhysicsValidation::ValidateVelocityConstraints(FCore_PhysicsValidationResult& Result)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        UPrimitiveComponent* PrimComp = Actor->GetRootComponent() ? 
            Cast<UPrimitiveComponent>(Actor->GetRootComponent()) : nullptr;
        
        if (PrimComp && PrimComp->GetBodyInstance())
        {
            FVector Velocity = PrimComp->GetPhysicsLinearVelocity();
            FVector AngularVelocity = PrimComp->GetPhysicsAngularVelocityInDegrees();
            
            if (Velocity.Size() > MaxVelocityThreshold)
            {
                Result.bIsValid = false;
                Result.ValidationMessages.Add(FString::Printf(TEXT("Actor %s exceeds velocity threshold: %.2f"), 
                    *Actor->GetName(), Velocity.Size()));
            }
            
            if (AngularVelocity.Size() > MaxAngularVelocityThreshold)
            {
                Result.bIsValid = false;
                Result.ValidationMessages.Add(FString::Printf(TEXT("Actor %s exceeds angular velocity threshold: %.2f"), 
                    *Actor->GetName(), AngularVelocity.Size()));
            }
        }
    }
}

void UCore_PhysicsValidation::ValidateCollisionSettings(FCore_PhysicsValidationResult& Result)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 InvalidCollisionCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (Component && Component->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                // Check for invalid collision channel settings
                if (Component->GetCollisionObjectType() == ECollisionChannel::ECC_MAX)
                {
                    InvalidCollisionCount++;
                }
            }
        }
    }
    
    if (InvalidCollisionCount > 0)
    {
        Result.ValidationMessages.Add(FString::Printf(TEXT("Found %d actors with invalid collision settings"), 
            InvalidCollisionCount));
    }
}

void UCore_PhysicsValidation::ValidatePhysicsMaterials(FCore_PhysicsValidationResult& Result)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 MissingMaterialCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (Component && Component->GetBodyInstance() && Component->GetBodyInstance()->bSimulatePhysics)
            {
                // Check if physics material is assigned
                if (!Component->GetBodyInstance()->GetSimplePhysicalMaterial())
                {
                    MissingMaterialCount++;
                }
            }
        }
    }
    
    if (MissingMaterialCount > 0)
    {
        Result.ValidationMessages.Add(FString::Printf(TEXT("Found %d physics actors without physics materials"), 
            MissingMaterialCount));
    }
}

FCore_PhysicsValidationResult UCore_PhysicsValidation::GetLatestValidationResult() const
{
    if (ValidationResults.Num() > 0)
    {
        return ValidationResults.Last();
    }
    
    FCore_PhysicsValidationResult EmptyResult;
    EmptyResult.bIsValid = false;
    EmptyResult.ValidationMessages.Add(TEXT("No validation results available"));
    return EmptyResult;
}

TArray<FCore_PhysicsValidationResult> UCore_PhysicsValidation::GetValidationHistory() const
{
    return ValidationResults;
}

void UCore_PhysicsValidation::SetValidationEnabled(bool bEnabled)
{
    bIsValidationEnabled = bEnabled;
    UE_LOG(LogPhysicsValidation, Log, TEXT("Physics validation %s"), 
        bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_PhysicsValidation::SetValidationThresholds(float MaxVelocity, float MaxAngularVelocity, 
    int32 MaxPhysicsActors, float MinFrameRate)
{
    MaxVelocityThreshold = MaxVelocity;
    MaxAngularVelocityThreshold = MaxAngularVelocity;
    MaxPhysicsActorsThreshold = MaxPhysicsActors;
    MinFrameRateThreshold = MinFrameRate;
    
    UE_LOG(LogPhysicsValidation, Log, TEXT("Updated validation thresholds"));
}

void UCore_PhysicsValidation::ForceValidationCheck()
{
    PerformValidationCheck();
}

bool UCore_PhysicsValidation::IsPhysicsSystemHealthy() const
{
    FCore_PhysicsValidationResult LatestResult = GetLatestValidationResult();
    return LatestResult.bIsValid && CurrentFrameRate >= MinFrameRateThreshold;
}