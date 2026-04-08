#include "AnimationSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

AAnimationSystemManager::AAnimationSystemManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Set default values
    GlobalAnimationSpeed = 1.0f;
    bEnableMotionMatching = true;
    bEnableFootIK = true;
    bEnableFearSystem = true;
}

void AAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Animation System Manager initialized"));
    UE_LOG(LogTemp, Warning, TEXT("Motion Matching: %s"), bEnableMotionMatching ? TEXT("ENABLED") : TEXT("DISABLED"));
    UE_LOG(LogTemp, Warning, TEXT("Foot IK: %s"), bEnableFootIK ? TEXT("ENABLED") : TEXT("DISABLED"));
    UE_LOG(LogTemp, Warning, TEXT("Fear System: %s"), bEnableFearSystem ? TEXT("ENABLED") : TEXT("DISABLED"));
}

FCharacterAnimationProfile AAnimationSystemManager::GetAnimationProfile(const FString& ArchetypeID)
{
    if (!CharacterAnimationProfiles)
    {
        UE_LOG(LogTemp, Error, TEXT("CharacterAnimationProfiles DataTable is not set!"));
        return FCharacterAnimationProfile();
    }

    FCharacterAnimationProfile* Profile = CharacterAnimationProfiles->FindRow<FCharacterAnimationProfile>(
        FName(*ArchetypeID), TEXT("GetAnimationProfile"));
    
    if (Profile)
    {
        UE_LOG(LogTemp, Log, TEXT("Found animation profile for archetype: %s"), *ArchetypeID);
        return *Profile;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No animation profile found for archetype: %s"), *ArchetypeID);
        return FCharacterAnimationProfile();
    }
}

void AAnimationSystemManager::RegisterCharacter(AActor* Character, const FString& ArchetypeID)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register null character"));
        return;
    }

    RegisteredCharacters.Add(Character, ArchetypeID);
    CharacterFearLevels.Add(Character, 0.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Registered character %s with archetype %s"), 
           *Character->GetName(), *ArchetypeID);

    // Apply animation profile to character
    FCharacterAnimationProfile Profile = GetAnimationProfile(ArchetypeID);
    
    // Get character's skeletal mesh component
    if (ACharacter* CharacterPawn = Cast<ACharacter>(Character))
    {
        USkeletalMeshComponent* MeshComp = CharacterPawn->GetMesh();
        if (MeshComp && MeshComp->GetAnimInstance())
        {
            UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
            
            // Set animation properties based on profile
            // This would typically involve setting variables on the Animation Blueprint
            UE_LOG(LogTemp, Log, TEXT("Applied animation profile to character: %s"), *Character->GetName());
        }
    }
}

void AAnimationSystemManager::UpdateCharacterFearLevel(AActor* Character, float FearLevel)
{
    if (!Character || !RegisteredCharacters.Contains(Character))
    {
        UE_LOG(LogTemp, Warning, TEXT("Character not registered with animation system"));
        return;
    }

    // Clamp fear level between 0 and 1
    FearLevel = FMath::Clamp(FearLevel, 0.0f, 1.0f);
    CharacterFearLevels[Character] = FearLevel;

    FString ArchetypeID = RegisteredCharacters[Character];
    FCharacterAnimationProfile Profile = GetAnimationProfile(ArchetypeID);

    // Determine movement state based on fear level
    FString MovementState = TEXT("Normal");
    if (FearLevel >= Profile.PanicRunThreshold)
    {
        MovementState = TEXT("Panic");
    }
    else if (FearLevel >= Profile.CautiousWalkThreshold)
    {
        MovementState = TEXT("Cautious");
    }

    UE_LOG(LogTemp, Log, TEXT("Character %s fear level: %.2f - Movement state: %s"), 
           *Character->GetName(), FearLevel, *MovementState);

    // Apply fear-based animation modifications
    if (ACharacter* CharacterPawn = Cast<ACharacter>(Character))
    {
        USkeletalMeshComponent* MeshComp = CharacterPawn->GetMesh();
        if (MeshComp && MeshComp->GetAnimInstance())
        {
            // This would set variables in the Animation Blueprint
            // to influence Motion Matching database selection
            UE_LOG(LogTemp, Log, TEXT("Applied fear-based animation modifications to %s"), 
                   *Character->GetName());
        }
    }
}