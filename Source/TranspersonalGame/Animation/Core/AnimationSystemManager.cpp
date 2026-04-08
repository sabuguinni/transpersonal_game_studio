#include "AnimationSystemManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AAnimationSystemManager::AAnimationSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default personality traits
    PersonalityTraits.Add(TEXT("Alertness"), 0.8f);
    PersonalityTraits.Add(TEXT("Aggression"), 0.3f);
    PersonalityTraits.Add(TEXT("Curiosity"), 0.6f);
    PersonalityTraits.Add(TEXT("Fearfulness"), 0.7f);
    PersonalityTraits.Add(TEXT("Playfulness"), 0.2f);
}

void AAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultDatabases();
    
    UE_LOG(LogTemp, Warning, TEXT("Animation System Manager initialized for Transpersonal Game"));
}

void AAnimationSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateAnimationLOD();
    OptimizeAnimationPerformance();
}

void AAnimationSystemManager::RegisterCharacter(AActor* Character, const FString& CharacterType)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("Attempted to register null character"));
        return;
    }

    RegisteredCharacters.Add(Character, CharacterType);
    
    // Apply default personality traits based on character type
    if (CharacterType == TEXT("Player"))
    {
        TMap<FString, float> PlayerTraits;
        PlayerTraits.Add(TEXT("Alertness"), 0.9f);
        PlayerTraits.Add(TEXT("Fearfulness"), 0.8f);
        PlayerTraits.Add(TEXT("Curiosity"), 0.7f);
        ApplyPersonalityToAnimation(Character, PlayerTraits);
    }
    else if (CharacterType.Contains(TEXT("Dinosaur")))
    {
        // Each dinosaur gets unique personality traits
        TMap<FString, float> DinosaurTraits;
        DinosaurTraits.Add(TEXT("Aggression"), FMath::RandRange(0.2f, 0.9f));
        DinosaurTraits.Add(TEXT("Alertness"), FMath::RandRange(0.5f, 1.0f));
        DinosaurTraits.Add(TEXT("Territoriality"), FMath::RandRange(0.3f, 0.8f));
        ApplyPersonalityToAnimation(Character, DinosaurTraits);
    }

    UE_LOG(LogTemp, Log, TEXT("Registered character %s as type %s"), 
           *Character->GetName(), *CharacterType);
}

void AAnimationSystemManager::UnregisterCharacter(AActor* Character)
{
    if (RegisteredCharacters.Contains(Character))
    {
        RegisteredCharacters.Remove(Character);
        UE_LOG(LogTemp, Log, TEXT("Unregistered character %s"), *Character->GetName());
    }
}

void AAnimationSystemManager::SetCharacterAnimationState(AActor* Character, const FString& NewState)
{
    if (!RegisteredCharacters.Contains(Character))
    {
        UE_LOG(LogTemp, Warning, TEXT("Character %s not registered with Animation System"), 
               *Character->GetName());
        return;
    }

    // Broadcast state change
    OnAnimationStateChanged.Broadcast(Character, NewState);
    
    UE_LOG(LogTemp, Log, TEXT("Character %s animation state changed to %s"), 
           *Character->GetName(), *NewState);
}

UPoseSearchDatabase* AAnimationSystemManager::GetAnimationDatabase(const FString& CharacterType, const FString& AnimationType)
{
    if (CharacterType == TEXT("Player"))
    {
        if (AnimationType == TEXT("Locomotion"))
            return PlayerLocomotionDatabase;
        else if (AnimationType == TEXT("Combat"))
            return PlayerCombatDatabase;
        else if (AnimationType == TEXT("Interaction"))
            return PlayerInteractionDatabase;
    }
    else if (CharacterType.Contains(TEXT("Dinosaur")))
    {
        if (AnimationType == TEXT("Locomotion"))
        {
            if (DinosaurLocomotionDatabases.Contains(CharacterType))
                return DinosaurLocomotionDatabases[CharacterType];
        }
        else if (AnimationType == TEXT("Behavior"))
        {
            if (DinosaurBehaviorDatabases.Contains(CharacterType))
                return DinosaurBehaviorDatabases[CharacterType];
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("No animation database found for %s - %s"), 
           *CharacterType, *AnimationType);
    return nullptr;
}

void AAnimationSystemManager::ApplyPersonalityToAnimation(AActor* Character, const TMap<FString, float>& Traits)
{
    if (!Character)
        return;

    USkeletalMeshComponent* MeshComp = Character->FindComponentByClass<USkeletalMeshComponent>();
    if (!MeshComp || !MeshComp->GetAnimInstance())
        return;

    // Store traits for this character (simplified - in real implementation would use character-specific storage)
    for (const auto& Trait : Traits)
    {
        PersonalityTraits.Add(Trait.Key, Trait.Value);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied personality traits to character %s"), *Character->GetName());
}

float AAnimationSystemManager::GetPersonalityInfluence(const FString& TraitName, float BaseValue)
{
    if (PersonalityTraits.Contains(TraitName))
    {
        float TraitValue = PersonalityTraits[TraitName];
        return BaseValue * TraitValue;
    }
    
    return BaseValue;
}

void AAnimationSystemManager::UpdateAnimationLOD()
{
    // Get player location for distance calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Update LOD for all registered characters
    for (const auto& CharacterPair : RegisteredCharacters)
    {
        AActor* Character = CharacterPair.Key;
        if (!Character)
            continue;

        float Distance = FVector::Dist(PlayerLocation, Character->GetActorLocation());
        
        USkeletalMeshComponent* MeshComp = Character->FindComponentByClass<USkeletalMeshComponent>();
        if (MeshComp)
        {
            // Adjust animation quality based on distance
            if (Distance > AnimationLODDistance * 2.0f)
            {
                // Lowest quality - minimal updates
                MeshComp->SetUpdateAnimationInEditor(false);
            }
            else if (Distance > AnimationLODDistance)
            {
                // Medium quality
                MeshComp->SetUpdateAnimationInEditor(true);
            }
            else
            {
                // High quality - full updates
                MeshComp->SetUpdateAnimationInEditor(true);
            }
        }
    }
}

void AAnimationSystemManager::OptimizeAnimationPerformance()
{
    // Count active animations
    int32 ActiveAnimations = 0;
    
    for (const auto& CharacterPair : RegisteredCharacters)
    {
        AActor* Character = CharacterPair.Key;
        if (!Character)
            continue;

        USkeletalMeshComponent* MeshComp = Character->FindComponentByClass<USkeletalMeshComponent>();
        if (MeshComp && MeshComp->IsPlaying())
        {
            ActiveAnimations++;
        }
    }

    // If we exceed max simultaneous animations, start culling distant ones
    if (ActiveAnimations > MaxSimultaneousAnimations)
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation performance optimization triggered: %d active animations"), 
               ActiveAnimations);
        // Implementation would prioritize closer characters
    }
}

bool AAnimationSystemManager::IsCharacterInRange(AActor* Character, float Range)
{
    if (!Character)
        return false;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return false;

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Character->GetActorLocation());
    return Distance <= Range;
}

void AAnimationSystemManager::InitializeDefaultDatabases()
{
    // Initialize default database mappings
    // These would be set up in Blueprint or loaded from data assets
    
    UE_LOG(LogTemp, Log, TEXT("Initializing default animation databases"));
    
    // Note: Actual database assets would be assigned in Blueprint
    // This is the C++ foundation for the system
}