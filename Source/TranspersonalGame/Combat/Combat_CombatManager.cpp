#include "Combat_CombatManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame/Character/TranspersonalCharacter.h"

UCombat_CombatManager::UCombat_CombatManager()
{
    CombatTimeoutDuration = 10.0f;
    DefaultDamageMultiplier = 1.0f;
    bEnableCombatLogging = true;
}

void UCombat_CombatManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat_CombatManager: Subsystem initialized"));
    
    // Start cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(CleanupTimerHandle, this, &UCombat_CombatManager::CleanupExpiredCombat, 5.0f, true);
    }
}

void UCombat_CombatManager::Deinitialize()
{
    // Clear all data
    RegisteredCombatants.Empty();
    ActorsInCombat.Empty();
    LastCombatTime.Empty();
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimerHandle);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Combat_CombatManager: Subsystem deinitialized"));
    
    Super::Deinitialize();
}

void UCombat_CombatManager::RegisterCombatant(AActor* Actor, ECombat_CombatantType CombatantType)
{
    if (!Actor)
        return;
    
    RegisteredCombatants.Add(Actor, CombatantType);
    
    if (bEnableCombatLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Combat_CombatManager: Registered combatant %s as type %d"), 
               *Actor->GetName(), static_cast<int32>(CombatantType));
    }
}

void UCombat_CombatManager::UnregisterCombatant(AActor* Actor)
{
    if (!Actor)
        return;
    
    RegisteredCombatants.Remove(Actor);
    ActorsInCombat.Remove(Actor);
    LastCombatTime.Remove(Actor);
    
    if (bEnableCombatLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Combat_CombatManager: Unregistered combatant %s"), *Actor->GetName());
    }
}

bool UCombat_CombatManager::DealDamage(AActor* Attacker, AActor* Target, float Damage, const FVector& HitLocation)
{
    if (!CanDealDamage(Attacker, Target))
        return false;
    
    // Apply damage multiplier
    float FinalDamage = Damage * DefaultDamageMultiplier;
    
    // Apply damage to target
    if (ATranspersonalCharacter* Character = Cast<ATranspersonalCharacter>(Target))
    {
        // Use character's health system
        Character->TakeDamage(FinalDamage, FDamageEvent(), nullptr, Attacker);
    }
    else
    {
        // Use generic damage system
        UGameplayStatics::ApplyPointDamage(Target, FinalDamage, HitLocation, FHitResult(), nullptr, Attacker, nullptr);
    }
    
    // Update combat state
    if (Attacker)
    {
        ActorsInCombat.Add(Attacker);
        LastCombatTime.Add(Attacker, GetWorld()->GetTimeSeconds());
    }
    
    if (Target)
    {
        ActorsInCombat.Add(Target);
        LastCombatTime.Add(Target, GetWorld()->GetTimeSeconds());
    }
    
    // Broadcast damage event
    BroadcastCombatEvent(OnDamageDealt, Attacker, Target, FinalDamage);
    
    if (bEnableCombatLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat_CombatManager: %s dealt %.2f damage to %s"), 
               Attacker ? *Attacker->GetName() : TEXT("Unknown"), 
               FinalDamage, 
               Target ? *Target->GetName() : TEXT("Unknown"));
    }
    
    return true;
}

void UCombat_CombatManager::StartCombat(AActor* Initiator, AActor* Target)
{
    if (!Initiator || !Target)
        return;
    
    ActorsInCombat.Add(Initiator);
    ActorsInCombat.Add(Target);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    LastCombatTime.Add(Initiator, CurrentTime);
    LastCombatTime.Add(Target, CurrentTime);
    
    // Broadcast combat started event
    BroadcastCombatEvent(OnCombatStarted, Initiator, Target, 0.0f);
    
    if (bEnableCombatLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat_CombatManager: Combat started between %s and %s"), 
               *Initiator->GetName(), *Target->GetName());
    }
}

void UCombat_CombatManager::EndCombat(AActor* Actor)
{
    if (!Actor)
        return;
    
    ActorsInCombat.Remove(Actor);
    LastCombatTime.Remove(Actor);
    
    // Broadcast combat ended event
    BroadcastCombatEvent(OnCombatEnded, Actor, nullptr, 0.0f);
    
    if (bEnableCombatLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Combat_CombatManager: Combat ended for %s"), *Actor->GetName());
    }
}

bool UCombat_CombatManager::IsInCombat(AActor* Actor) const
{
    return ActorsInCombat.Contains(Actor);
}

TArray<AActor*> UCombat_CombatManager::GetCombatantsInRange(const FVector& Location, float Radius, ECombat_CombatantType CombatantType) const
{
    TArray<AActor*> Result;
    
    for (const auto& Pair : RegisteredCombatants)
    {
        AActor* Actor = Pair.Key;
        ECombat_CombatantType ActorType = Pair.Value;
        
        if (!Actor || !IsValid(Actor))
            continue;
        
        // Check type filter
        if (CombatantType != ECombat_CombatantType::Any && ActorType != CombatantType)
            continue;
        
        // Check distance
        float Distance = FVector::Dist(Actor->GetActorLocation(), Location);
        if (Distance <= Radius)
        {
            Result.Add(Actor);
        }
    }
    
    return Result;
}

AActor* UCombat_CombatManager::FindNearestEnemy(AActor* Searcher, float MaxRange) const
{
    if (!Searcher)
        return nullptr;
    
    ECombat_CombatantType SearcherType = GetCombatantType(Searcher);
    if (SearcherType == ECombat_CombatantType::Any)
        return nullptr;
    
    AActor* NearestEnemy = nullptr;
    float NearestDistance = MaxRange;
    FVector SearcherLocation = Searcher->GetActorLocation();
    
    for (const auto& Pair : RegisteredCombatants)
    {
        AActor* Actor = Pair.Key;
        ECombat_CombatantType ActorType = Pair.Value;
        
        if (!Actor || Actor == Searcher || !IsValid(Actor))
            continue;
        
        // Check if this is an enemy type
        bool bIsEnemy = false;
        if (SearcherType == ECombat_CombatantType::Player && ActorType == ECombat_CombatantType::Dinosaur)
            bIsEnemy = true;
        else if (SearcherType == ECombat_CombatantType::Dinosaur && ActorType == ECombat_CombatantType::Player)
            bIsEnemy = true;
        
        if (!bIsEnemy)
            continue;
        
        // Check distance
        float Distance = FVector::Dist(Actor->GetActorLocation(), SearcherLocation);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestEnemy = Actor;
        }
    }
    
    return NearestEnemy;
}

int32 UCombat_CombatManager::GetActiveCombatCount() const
{
    return ActorsInCombat.Num();
}

TArray<AActor*> UCombat_CombatManager::GetAllCombatants() const
{
    TArray<AActor*> Result;
    RegisteredCombatants.GetKeys(Result);
    return Result;
}

ECombat_CombatantType UCombat_CombatManager::GetCombatantType(AActor* Actor) const
{
    if (const ECombat_CombatantType* Type = RegisteredCombatants.Find(Actor))
    {
        return *Type;
    }
    return ECombat_CombatantType::Any;
}

void UCombat_CombatManager::CleanupExpiredCombat()
{
    if (!GetWorld())
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    TArray<AActor*> ExpiredActors;
    
    for (const auto& Pair : LastCombatTime)
    {
        AActor* Actor = Pair.Key;
        float LastTime = Pair.Value;
        
        if (CurrentTime - LastTime > CombatTimeoutDuration)
        {
            ExpiredActors.Add(Actor);
        }
    }
    
    // Remove expired actors from combat
    for (AActor* Actor : ExpiredActors)
    {
        EndCombat(Actor);
    }
}

bool UCombat_CombatManager::CanDealDamage(AActor* Attacker, AActor* Target) const
{
    if (!Attacker || !Target)
        return false;
    
    if (Attacker == Target)
        return false;
    
    // Check if both are registered combatants
    if (!RegisteredCombatants.Contains(Attacker) || !RegisteredCombatants.Contains(Target))
        return false;
    
    return true;
}

void UCombat_CombatManager::BroadcastCombatEvent(const FOnCombatEvent& Event, AActor* Attacker, AActor* Target, float Damage)
{
    if (Event.IsBound())
    {
        Event.Broadcast(Attacker, Target, Damage);
    }
}