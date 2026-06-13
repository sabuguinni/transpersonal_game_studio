#include "Core_RagdollSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsSettings.h"

UCore_RagdollComponent::UCore_RagdollComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    MaxRagdollTime = 10.0f;
    PhysicsBlendWeight = 1.0f;
    bAutoDeactivate = true;
    bIsRagdollActive = false;
    RagdollTimer = 0.0f;
}

void UCore_RagdollComponent::BeginPlay()
{
    Super::BeginPlay();
    
    FindSkeletalMeshComponent();
    
    if (UCore_RagdollManager* Manager = UCore_RagdollManager::GetRagdollManager(this))
    {
        Manager->RegisterRagdollComponent(this);
    }
}

void UCore_RagdollComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsRagdollActive)
    {
        RagdollTimer += DeltaTime;
        
        if (bAutoDeactivate && RagdollTimer >= MaxRagdollTime)
        {
            DeactivateRagdoll();
        }
    }
}

void UCore_RagdollComponent::FindSkeletalMeshComponent()
{
    if (AActor* Owner = GetOwner())
    {
        SkeletalMeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
        
        if (!SkeletalMeshComp)
        {
            if (ACharacter* Character = Cast<ACharacter>(Owner))
            {
                SkeletalMeshComp = Character->GetMesh();
            }
        }
    }
}

void UCore_RagdollComponent::ActivateRagdoll(const FCore_RagdollData& RagdollData)
{
    if (!SkeletalMeshComp || bIsRagdollActive)
    {
        return;
    }
    
    CurrentRagdollData = RagdollData;
    bIsRagdollActive = true;
    RagdollTimer = 0.0f;
    
    SetComponentTickEnabled(true);
    
    BlendToRagdoll();
    
    if (RagdollData.HitBoneName != NAME_None && RagdollData.HitDirection.SizeSquared() > 0.0f)
    {
        ApplyImpulseToRagdoll(RagdollData.HitDirection * RagdollData.ImpactForce, RagdollData.HitBoneName);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll activated for %s"), *GetOwner()->GetName());
}

void UCore_RagdollComponent::DeactivateRagdoll()
{
    if (!bIsRagdollActive)
    {
        return;
    }
    
    bIsRagdollActive = false;
    SetComponentTickEnabled(false);
    
    BlendFromRagdoll();
    
    if (UCore_RagdollManager* Manager = UCore_RagdollManager::GetRagdollManager(this))
    {
        Manager->UnregisterRagdollComponent(this);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll deactivated for %s"), *GetOwner()->GetName());
}

bool UCore_RagdollComponent::IsRagdollActive() const
{
    return bIsRagdollActive;
}

void UCore_RagdollComponent::ApplyImpulseToRagdoll(const FVector& Impulse, const FName& BoneName)
{
    if (!SkeletalMeshComp || !bIsRagdollActive)
    {
        return;
    }
    
    if (BoneName != NAME_None)
    {
        SkeletalMeshComp->AddImpulseAtLocation(Impulse, SkeletalMeshComp->GetBoneLocation(BoneName), BoneName);
    }
    else
    {
        SkeletalMeshComp->AddImpulse(Impulse);
    }
}

void UCore_RagdollComponent::SetRagdollPhysicsBlend(float BlendWeight)
{
    PhysicsBlendWeight = FMath::Clamp(BlendWeight, 0.0f, 1.0f);
    
    if (SkeletalMeshComp)
    {
        SkeletalMeshComp->SetAllBodiesPhysicsBlendWeight(PhysicsBlendWeight);
    }
}

void UCore_RagdollComponent::BlendToRagdoll()
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMeshComp->SetSimulatePhysics(true);
    
    if (CurrentRagdollData.bUsePhysicsBlend)
    {
        SkeletalMeshComp->SetAllBodiesPhysicsBlendWeight(PhysicsBlendWeight);
    }
    else
    {
        SkeletalMeshComp->SetAllBodiesSimulatePhysics(true);
    }
    
    if (AActor* Owner = GetOwner())
    {
        Owner->SetActorEnableCollision(true);
    }
}

void UCore_RagdollComponent::BlendFromRagdoll()
{
    if (!SkeletalMeshComp)
    {
        return;
    }
    
    SkeletalMeshComp->SetSimulatePhysics(false);
    SkeletalMeshComp->SetAllBodiesSimulatePhysics(false);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    if (UAnimInstance* AnimInstance = SkeletalMeshComp->GetAnimInstance())
    {
        AnimInstance->Montage_Stop(CurrentRagdollData.BlendTime);
    }
}

void UCore_RagdollManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    GlobalMaxRagdollTime = 15.0f;
    bGlobalAutoDeactivate = true;
    MaxSimultaneousRagdolls = 20;
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollManager initialized"));
}

void UCore_RagdollManager::Deinitialize()
{
    DeactivateAllRagdolls();
    ActiveRagdolls.Empty();
    
    Super::Deinitialize();
    
    UE_LOG(LogTemp, Log, TEXT("Core_RagdollManager deinitialized"));
}

UCore_RagdollManager* UCore_RagdollManager::GetRagdollManager(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UCore_RagdollManager>();
        }
    }
    return nullptr;
}

void UCore_RagdollManager::RegisterRagdollComponent(UCore_RagdollComponent* Component)
{
    if (Component && !ActiveRagdolls.Contains(Component))
    {
        if (ActiveRagdolls.Num() >= MaxSimultaneousRagdolls)
        {
            CleanupInvalidComponents();
            
            if (ActiveRagdolls.Num() >= MaxSimultaneousRagdolls)
            {
                UE_LOG(LogTemp, Warning, TEXT("Maximum ragdoll limit reached (%d). Cannot register new ragdoll."), MaxSimultaneousRagdolls);
                return;
            }
        }
        
        ActiveRagdolls.Add(Component);
        UE_LOG(LogTemp, Log, TEXT("Registered ragdoll component. Active count: %d"), ActiveRagdolls.Num());
    }
}

void UCore_RagdollManager::UnregisterRagdollComponent(UCore_RagdollComponent* Component)
{
    if (Component)
    {
        ActiveRagdolls.Remove(Component);
        UE_LOG(LogTemp, Log, TEXT("Unregistered ragdoll component. Active count: %d"), ActiveRagdolls.Num());
    }
}

void UCore_RagdollManager::DeactivateAllRagdolls()
{
    for (UCore_RagdollComponent* Component : ActiveRagdolls)
    {
        if (IsValid(Component))
        {
            Component->DeactivateRagdoll();
        }
    }
    
    ActiveRagdolls.Empty();
    UE_LOG(LogTemp, Log, TEXT("Deactivated all ragdolls"));
}

int32 UCore_RagdollManager::GetActiveRagdollCount() const
{
    return ActiveRagdolls.Num();
}

void UCore_RagdollManager::SetGlobalRagdollSettings(float MaxTime, bool bAutoDeactivate)
{
    GlobalMaxRagdollTime = MaxTime;
    bGlobalAutoDeactivate = bAutoDeactivate;
    
    for (UCore_RagdollComponent* Component : ActiveRagdolls)
    {
        if (IsValid(Component))
        {
            Component->MaxRagdollTime = GlobalMaxRagdollTime;
            Component->bAutoDeactivate = bGlobalAutoDeactivate;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Updated global ragdoll settings: MaxTime=%.1f, AutoDeactivate=%s"), 
           MaxTime, bAutoDeactivate ? TEXT("true") : TEXT("false"));
}

void UCore_RagdollManager::CleanupInvalidComponents()
{
    ActiveRagdolls.RemoveAll([](UCore_RagdollComponent* Component)
    {
        return !IsValid(Component);
    });
}