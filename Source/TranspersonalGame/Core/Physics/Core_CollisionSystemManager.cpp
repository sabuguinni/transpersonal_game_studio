#include "Core_CollisionSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Components/PrimitiveComponent.h"

UCore_CollisionComponent::UCore_CollisionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bEnableCollisionLogging = true;
    CollisionDamageMultiplier = 1.0f;
    MinImpactForceForDamage = 100.0f;
    LastCollisionTime = 0.0f;
    
    CollisionProfile.ProfileName = FName("Default");
    CollisionProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    CollisionProfile.ObjectType = ECollisionObjectType::WorldDynamic;
}

void UCore_CollisionComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (UCore_CollisionSystemManager* Manager = UCore_CollisionSystemManager::GetCollisionSystemManager(this))
    {
        Manager->RegisterCollisionComponent(this);
    }
}

void UCore_CollisionComponent::SetCollisionProfile(const FCore_CollisionProfile& NewProfile)
{
    CollisionProfile = NewProfile;
    
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->GetRootComponent())
        {
            if (UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(PrimComp))
            {
                PrimComponent->SetCollisionEnabled(CollisionProfile.CollisionEnabled);
                PrimComponent->SetCollisionObjectType(CollisionProfile.ObjectType);
            }
        }
    }
}

void UCore_CollisionComponent::EnableCollision(bool bEnable)
{
    if (AActor* Owner = GetOwner())
    {
        if (UPrimitiveComponent* PrimComp = Owner->GetRootComponent())
        {
            if (UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(PrimComp))
            {
                ECollisionEnabled::Type NewEnabled = bEnable ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision;
                PrimComponent->SetCollisionEnabled(NewEnabled);
            }
        }
    }
}

void UCore_CollisionComponent::RegisterCollisionEvent(const FCore_CollisionEvent& Event)
{
    if (!bEnableCollisionLogging)
    {
        return;
    }
    
    RecentCollisionEvents.Add(Event);
    LastCollisionTime = Event.TimeStamp;
    
    // Keep only recent events (last 10)
    if (RecentCollisionEvents.Num() > 10)
    {
        RecentCollisionEvents.RemoveAt(0);
    }
    
    // Trigger Blueprint event
    OnCollisionEventReceived(Event);
    
    // Forward to system manager
    if (UCore_CollisionSystemManager* Manager = UCore_CollisionSystemManager::GetCollisionSystemManager(this))
    {
        Manager->ProcessCollisionEvent(Event);
    }
}

void UCore_CollisionSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bGlobalCollisionEnabled = true;
    MaxHistoryTime = 30.0f; // Keep 30 seconds of collision history
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystemManager initialized"));
    
    // Set up periodic cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UCore_CollisionSystemManager::CleanupOldEvents,
            5.0f, // Every 5 seconds
            true  // Looping
        );
    }
}

void UCore_CollisionSystemManager::Deinitialize()
{
    RegisteredComponents.Empty();
    GlobalCollisionHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionSystemManager deinitialized"));
    
    Super::Deinitialize();
}

UCore_CollisionSystemManager* UCore_CollisionSystemManager::GetCollisionSystemManager(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UCore_CollisionSystemManager>();
    }
    return nullptr;
}

void UCore_CollisionSystemManager::RegisterCollisionComponent(UCore_CollisionComponent* Component)
{
    if (Component && !RegisteredComponents.Contains(Component))
    {
        RegisteredComponents.Add(Component);
        UE_LOG(LogTemp, Log, TEXT("Registered collision component for actor: %s"), 
               Component->GetOwner() ? *Component->GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void UCore_CollisionSystemManager::UnregisterCollisionComponent(UCore_CollisionComponent* Component)
{
    if (Component)
    {
        RegisteredComponents.Remove(Component);
        UE_LOG(LogTemp, Log, TEXT("Unregistered collision component for actor: %s"), 
               Component->GetOwner() ? *Component->GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void UCore_CollisionSystemManager::ProcessCollisionEvent(const FCore_CollisionEvent& Event)
{
    if (!bGlobalCollisionEnabled)
    {
        return;
    }
    
    // Add to global history
    GlobalCollisionHistory.Add(Event);
    
    // Log significant collisions
    if (Event.ImpactForce > 500.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("High impact collision: %f force between %s and %s"),
               Event.ImpactForce,
               Event.ActorA ? *Event.ActorA->GetName() : TEXT("Unknown"),
               Event.ActorB ? *Event.ActorB->GetName() : TEXT("Unknown"));
    }
    
    // Validate components periodically
    static float LastValidationTime = 0.0f;
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - LastValidationTime > 10.0f)
    {
        ValidateComponents();
        LastValidationTime = CurrentTime;
    }
}

TArray<FCore_CollisionEvent> UCore_CollisionSystemManager::GetRecentCollisionEvents(float TimeWindow)
{
    TArray<FCore_CollisionEvent> RecentEvents;
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    float CutoffTime = CurrentTime - TimeWindow;
    
    for (const FCore_CollisionEvent& Event : GlobalCollisionHistory)
    {
        if (Event.TimeStamp >= CutoffTime)
        {
            RecentEvents.Add(Event);
        }
    }
    
    return RecentEvents;
}

void UCore_CollisionSystemManager::SetGlobalCollisionEnabled(bool bEnabled)
{
    bGlobalCollisionEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Global collision system %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_CollisionSystemManager::ClearCollisionHistory()
{
    GlobalCollisionHistory.Empty();
    UE_LOG(LogTemp, Log, TEXT("Collision history cleared"));
}

int32 UCore_CollisionSystemManager::GetActiveCollisionComponentCount() const
{
    return RegisteredComponents.Num();
}

void UCore_CollisionSystemManager::CleanupOldEvents()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float CutoffTime = CurrentTime - MaxHistoryTime;
    
    // Remove old events
    GlobalCollisionHistory.RemoveAll([CutoffTime](const FCore_CollisionEvent& Event)
    {
        return Event.TimeStamp < CutoffTime;
    });
    
    // Clean up null components
    ValidateComponents();
}

void UCore_CollisionSystemManager::ValidateComponents()
{
    RegisteredComponents.RemoveAll([](UCore_CollisionComponent* Component)
    {
        return !IsValid(Component) || !IsValid(Component->GetOwner());
    });
}