#include "Core_RagdollManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "TimerManager.h"

UCore_RagdollManager::UCore_RagdollManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize ragdoll settings
    MaxActiveRagdolls = 20;
    RagdollLifetime = 30.0f;
    BlendInTime = 0.2f;
    BlendOutTime = 0.5f;
    
    bAutoCleanupRagdolls = true;
    bEnableRagdollCollision = true;
    bEnableRagdollConstraints = true;
    
    // Performance settings
    RagdollUpdateRate = 30.0f;  // Lower than physics rate for performance
    MaxRagdollDistance = 5000.0f;  // Cull distant ragdolls
    
    CurrentActiveRagdolls = 0;
    LastCleanupTime = 0.0f;
    CleanupInterval = 5.0f;  // Cleanup every 5 seconds
}

void UCore_RagdollManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core Ragdoll Manager - BeginPlay"));
    
    // Initialize ragdoll pools
    InitializeRagdollPools();
    
    // Set up cleanup timer
    if (bAutoCleanupRagdolls)
    {
        GetWorld()->GetTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &UCore_RagdollManager::CleanupExpiredRagdolls,
            CleanupInterval,
            true
        );
    }
}

void UCore_RagdollManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update active ragdolls
    UpdateActiveRagdolls(DeltaTime);
    
    // Performance monitoring
    MonitorRagdollPerformance();
}

void UCore_RagdollManager::InitializeRagdollPools()
{
    // Initialize different ragdoll types for different creatures
    
    // Human/Character ragdoll settings
    FCore_RagdollTypeSettings HumanSettings;
    HumanSettings.RagdollType = ECore_RagdollType::Human;
    HumanSettings.MaxInstances = 5;
    HumanSettings.DefaultLifetime = 30.0f;
    HumanSettings.BlendInTime = 0.2f;
    HumanSettings.BlendOutTime = 0.5f;
    HumanSettings.bEnableCollision = true;
    HumanSettings.bEnableConstraints = true;
    HumanSettings.LinearDamping = 0.1f;
    HumanSettings.AngularDamping = 0.1f;
    RagdollTypeSettings.Add(ECore_RagdollType::Human, HumanSettings);
    
    // Small Dinosaur ragdoll settings (Raptors)
    FCore_RagdollTypeSettings SmallDinoSettings;
    SmallDinoSettings.RagdollType = ECore_RagdollType::SmallDinosaur;
    SmallDinoSettings.MaxInstances = 8;
    SmallDinoSettings.DefaultLifetime = 25.0f;
    SmallDinoSettings.BlendInTime = 0.15f;
    SmallDinoSettings.BlendOutTime = 0.4f;
    SmallDinoSettings.bEnableCollision = true;
    SmallDinoSettings.bEnableConstraints = true;
    SmallDinoSettings.LinearDamping = 0.05f;
    SmallDinoSettings.AngularDamping = 0.05f;
    RagdollTypeSettings.Add(ECore_RagdollType::SmallDinosaur, SmallDinoSettings);
    
    // Large Dinosaur ragdoll settings (T-Rex, Brachiosaurus)
    FCore_RagdollTypeSettings LargeDinoSettings;
    LargeDinoSettings.RagdollType = ECore_RagdollType::LargeDinosaur;
    LargeDinoSettings.MaxInstances = 3;
    LargeDinoSettings.DefaultLifetime = 45.0f;
    LargeDinoSettings.BlendInTime = 0.3f;
    LargeDinoSettings.BlendOutTime = 0.8f;
    LargeDinoSettings.bEnableCollision = true;
    LargeDinoSettings.bEnableConstraints = true;
    LargeDinoSettings.LinearDamping = 0.2f;
    LargeDinoSettings.AngularDamping = 0.2f;
    RagdollTypeSettings.Add(ECore_RagdollType::LargeDinosaur, LargeDinoSettings);
    
    // Generic creature ragdoll settings
    FCore_RagdollTypeSettings GenericSettings;
    GenericSettings.RagdollType = ECore_RagdollType::Generic;
    GenericSettings.MaxInstances = 4;
    GenericSettings.DefaultLifetime = 20.0f;
    GenericSettings.BlendInTime = 0.2f;
    GenericSettings.BlendOutTime = 0.5f;
    GenericSettings.bEnableCollision = true;
    GenericSettings.bEnableConstraints = true;
    GenericSettings.LinearDamping = 0.1f;
    GenericSettings.AngularDamping = 0.1f;
    RagdollTypeSettings.Add(ECore_RagdollType::Generic, GenericSettings);
    
    UE_LOG(LogTemp, Warning, TEXT("Ragdoll pools initialized - %d ragdoll types configured"), RagdollTypeSettings.Num());
}

bool UCore_RagdollManager::EnableRagdollForActor(AActor* Actor, ECore_RagdollType RagdollType, float CustomLifetime)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Error, TEXT("EnableRagdollForActor: Actor is null"));
        return false;
    }
    
    if (CurrentActiveRagdolls >= MaxActiveRagdolls)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnableRagdollForActor: Max ragdolls reached (%d)"), MaxActiveRagdolls);
        return false;
    }
    
    // Check if we have settings for this ragdoll type
    if (!RagdollTypeSettings.Contains(RagdollType))
    {
        UE_LOG(LogTemp, Warning, TEXT("EnableRagdollForActor: No settings for ragdoll type %d"), (int32)RagdollType);
        RagdollType = ECore_RagdollType::Generic;
    }
    
    const FCore_RagdollTypeSettings& Settings = RagdollTypeSettings[RagdollType];
    
    // Check type-specific limits
    int32 TypeCount = 0;
    for (const FCore_ActiveRagdoll& ActiveRagdoll : ActiveRagdolls)
    {
        if (ActiveRagdoll.RagdollType == RagdollType)
        {
            TypeCount++;
        }
    }
    
    if (TypeCount >= Settings.MaxInstances)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnableRagdollForActor: Max instances for type %d reached (%d)"), 
               (int32)RagdollType, Settings.MaxInstances);
        return false;
    }
    
    // Find skeletal mesh component
    USkeletalMeshComponent* SkelMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkelMeshComp)
    {
        UE_LOG(LogTemp, Error, TEXT("EnableRagdollForActor: No skeletal mesh component found on %s"), *Actor->GetName());
        return false;
    }
    
    // Check if physics asset exists
    if (!SkelMeshComp->GetPhysicsAsset())
    {
        UE_LOG(LogTemp, Error, TEXT("EnableRagdollForActor: No physics asset found on %s"), *Actor->GetName());
        return false;
    }
    
    // Create ragdoll entry
    FCore_ActiveRagdoll NewRagdoll;
    NewRagdoll.Actor = Actor;
    NewRagdoll.SkeletalMeshComponent = SkelMeshComp;
    NewRagdoll.RagdollType = RagdollType;
    NewRagdoll.StartTime = GetWorld()->GetTimeSeconds();
    NewRagdoll.Lifetime = (CustomLifetime > 0.0f) ? CustomLifetime : Settings.DefaultLifetime;
    NewRagdoll.BlendInTime = Settings.BlendInTime;
    NewRagdoll.BlendOutTime = Settings.BlendOutTime;
    NewRagdoll.bIsBlendingIn = true;
    NewRagdoll.bIsBlendingOut = false;
    NewRagdoll.BlendAlpha = 0.0f;
    
    // Store original animation state
    if (UAnimInstance* AnimInstance = SkelMeshComp->GetAnimInstance())
    {
        NewRagdoll.OriginalAnimInstance = AnimInstance;
    }
    
    // Enable ragdoll physics
    SkelMeshComp->SetSimulatePhysics(true);
    
    if (Settings.bEnableCollision)
    {
        SkelMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SkelMeshComp->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
    }
    
    // Apply damping settings
    SkelMeshComp->SetLinearDamping(Settings.LinearDamping);
    SkelMeshComp->SetAngularDamping(Settings.AngularDamping);
    
    // Add to active ragdolls
    ActiveRagdolls.Add(NewRagdoll);
    CurrentActiveRagdolls++;
    
    UE_LOG(LogTemp, Warning, TEXT("Ragdoll enabled for %s (Type: %d, Lifetime: %.1f)"), 
           *Actor->GetName(), (int32)RagdollType, NewRagdoll.Lifetime);
    
    return true;
}

bool UCore_RagdollManager::DisableRagdollForActor(AActor* Actor)
{
    if (!Actor)
        return false;
    
    // Find the ragdoll in our active list
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        if (ActiveRagdolls[i].Actor == Actor)
        {
            FCore_ActiveRagdoll& Ragdoll = ActiveRagdolls[i];
            
            // Disable physics
            if (Ragdoll.SkeletalMeshComponent && IsValid(Ragdoll.SkeletalMeshComponent))
            {
                Ragdoll.SkeletalMeshComponent->SetSimulatePhysics(false);
                Ragdoll.SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                
                // Restore animation if we had one
                if (Ragdoll.OriginalAnimInstance && IsValid(Ragdoll.OriginalAnimInstance))
                {
                    // Animation restoration would go here
                }
            }
            
            // Remove from active list
            ActiveRagdolls.RemoveAt(i);
            CurrentActiveRagdolls--;
            
            UE_LOG(LogTemp, Warning, TEXT("Ragdoll disabled for %s"), *Actor->GetName());
            return true;
        }
    }
    
    return false;
}

void UCore_RagdollManager::UpdateActiveRagdolls(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        FCore_ActiveRagdoll& Ragdoll = ActiveRagdolls[i];
        
        // Check if actor is still valid
        if (!IsValid(Ragdoll.Actor) || !IsValid(Ragdoll.SkeletalMeshComponent))
        {
            ActiveRagdolls.RemoveAt(i);
            CurrentActiveRagdolls--;
            continue;
        }
        
        float ElapsedTime = CurrentTime - Ragdoll.StartTime;
        
        // Update blend states
        if (Ragdoll.bIsBlendingIn)
        {
            Ragdoll.BlendAlpha = FMath::Clamp(ElapsedTime / Ragdoll.BlendInTime, 0.0f, 1.0f);
            if (Ragdoll.BlendAlpha >= 1.0f)
            {
                Ragdoll.bIsBlendingIn = false;
            }
        }
        else if (Ragdoll.bIsBlendingOut)
        {
            float BlendOutStartTime = Ragdoll.Lifetime - Ragdoll.BlendOutTime;
            float BlendOutElapsed = ElapsedTime - BlendOutStartTime;
            Ragdoll.BlendAlpha = 1.0f - FMath::Clamp(BlendOutElapsed / Ragdoll.BlendOutTime, 0.0f, 1.0f);
        }
        else
        {
            // Check if we should start blending out
            if (ElapsedTime >= (Ragdoll.Lifetime - Ragdoll.BlendOutTime))
            {
                Ragdoll.bIsBlendingOut = true;
            }
        }
        
        // Check if ragdoll has expired
        if (ElapsedTime >= Ragdoll.Lifetime)
        {
            DisableRagdollForActor(Ragdoll.Actor);
        }
    }
}

void UCore_RagdollManager::CleanupExpiredRagdolls()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        const FCore_ActiveRagdoll& Ragdoll = ActiveRagdolls[i];
        float ElapsedTime = CurrentTime - Ragdoll.StartTime;
        
        if (ElapsedTime >= Ragdoll.Lifetime)
        {
            DisableRagdollForActor(Ragdoll.Actor);
        }
    }
    
    LastCleanupTime = CurrentTime;
}

void UCore_RagdollManager::MonitorRagdollPerformance()
{
    // Performance monitoring
    if (CurrentActiveRagdolls > MaxActiveRagdolls * 0.8f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Ragdoll count high: %d/%d - Consider cleanup"), 
               CurrentActiveRagdolls, MaxActiveRagdolls);
    }
    
    // Distance culling for performance
    if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
        {
            const FCore_ActiveRagdoll& Ragdoll = ActiveRagdolls[i];
            if (IsValid(Ragdoll.Actor))
            {
                float Distance = FVector::Dist(PlayerLocation, Ragdoll.Actor->GetActorLocation());
                if (Distance > MaxRagdollDistance)
                {
                    // Disable distant ragdolls for performance
                    DisableRagdollForActor(Ragdoll.Actor);
                }
            }
        }
    }
}

void UCore_RagdollManager::ForceCleanupAllRagdolls()
{
    UE_LOG(LogTemp, Warning, TEXT("Force cleanup - Disabling all %d active ragdolls"), ActiveRagdolls.Num());
    
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        DisableRagdollForActor(ActiveRagdolls[i].Actor);
    }
    
    ActiveRagdolls.Empty();
    CurrentActiveRagdolls = 0;
}

int32 UCore_RagdollManager::GetActiveRagdollCount() const
{
    return CurrentActiveRagdolls;
}

int32 UCore_RagdollManager::GetActiveRagdollCountByType(ECore_RagdollType RagdollType) const
{
    int32 Count = 0;
    for (const FCore_ActiveRagdoll& Ragdoll : ActiveRagdolls)
    {
        if (Ragdoll.RagdollType == RagdollType)
        {
            Count++;
        }
    }
    return Count;
}

bool UCore_RagdollManager::IsActorRagdolled(AActor* Actor) const
{
    if (!Actor)
        return false;
        
    for (const FCore_ActiveRagdoll& Ragdoll : ActiveRagdolls)
    {
        if (Ragdoll.Actor == Actor)
        {
            return true;
        }
    }
    
    return false;
}