#include "AnimationSystemManager.h"
#include "Engine/Engine.h"

AAnimationSystemManager::AAnimationSystemManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Configuração padrão do protagonista
    ProtagonistProfile.CharacterName = TEXT("Dr. Paleontologist");
    ProtagonistProfile.CautiousnessFactor = 0.8f; // Muito cauteloso por padrão
    ProtagonistProfile.FatigueLevel = 0.0f;
    ProtagonistProfile.FearLevel = 0.3f; // Medo base constante

    // Configuração de dinossauros comuns
    FDinosaurAnimationProfile TriceratopsProfile;
    TriceratopsProfile.SpeciesName = TEXT("Triceratops");
    TriceratopsProfile.Size = 2.5f;
    TriceratopsProfile.Aggressiveness = 0.3f;
    TriceratopsProfile.bIsHerbivore = true;
    DinosaurProfiles.Add(TriceratopsProfile);

    FDinosaurAnimationProfile VelociraprorProfile;
    VelociraprorProfile.SpeciesName = TEXT("Velociraptor");
    VelociraprorProfile.Size = 0.8f;
    VelociraprorProfile.Aggressiveness = 0.9f;
    VelociraprorProfile.bIsHerbivore = false;
    DinosaurProfiles.Add(VelociraprorProfile);

    FDinosaurAnimationProfile TRexProfile;
    TRexProfile.SpeciesName = TEXT("TyrannosaurusRex");
    TRexProfile.Size = 4.0f;
    TRexProfile.Aggressiveness = 1.0f;
    TRexProfile.bIsHerbivore = false;
    DinosaurProfiles.Add(TRexProfile);

    FDinosaurAnimationProfile BrachiosaurusProfile;
    BrachiosaurusProfile.SpeciesName = TEXT("Brachiosaurus");
    BrachiosaurusProfile.Size = 5.0f;
    BrachiosaurusProfile.Aggressiveness = 0.1f;
    BrachiosaurusProfile.bIsHerbivore = true;
    DinosaurProfiles.Add(BrachiosaurusProfile);

    FDinosaurAnimationProfile ParasaurolophusProfile;
    ParasaurolophusProfile.SpeciesName = TEXT("Parasaurolophus");
    ParasaurolophusProfile.Size = 1.5f;
    ParasaurolophusProfile.Aggressiveness = 0.2f;
    ParasaurolophusProfile.bIsHerbivore = true;
    DinosaurProfiles.Add(ParasaurolophusProfile);
}

void AAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Animation System Manager initialized with %d dinosaur profiles"), DinosaurProfiles.Num());
}

FDinosaurAnimationProfile AAnimationSystemManager::GetDinosaurProfile(const FString& SpeciesName) const
{
    for (const FDinosaurAnimationProfile& Profile : DinosaurProfiles)
    {
        if (Profile.SpeciesName == SpeciesName)
        {
            return Profile;
        }
    }
    
    // Retorna perfil padrão se não encontrar
    FDinosaurAnimationProfile DefaultProfile;
    DefaultProfile.SpeciesName = SpeciesName;
    return DefaultProfile;
}

void AAnimationSystemManager::UpdateCharacterFearLevel(float NewFearLevel)
{
    ProtagonistProfile.FearLevel = FMath::Clamp(NewFearLevel, 0.0f, 1.0f);
    
    // Medo afeta a cautela
    ProtagonistProfile.CautiousnessFactor = FMath::Lerp(0.5f, 1.0f, ProtagonistProfile.FearLevel);
}

void AAnimationSystemManager::UpdateCharacterFatigue(float NewFatigueLevel)
{
    ProtagonistProfile.FatigueLevel = FMath::Clamp(NewFatigueLevel, 0.0f, 1.0f);
}

UPoseSearchDatabase* AAnimationSystemManager::GetMotionMatchingDatabase(ECharacterMovementState MovementState) const
{
    // Por agora retorna a database principal, mas pode ser expandido para databases específicas por estado
    return ProtagonistProfile.MotionMatchingDatabase;
}

UPoseSearchDatabase* AAnimationSystemManager::GetDinosaurDatabase(const FString& SpeciesName, EDinosaurBehaviorState BehaviorState) const
{
    FDinosaurAnimationProfile Profile = GetDinosaurProfile(SpeciesName);
    
    switch (BehaviorState)
    {
    case EDinosaurBehaviorState::Idle:
    case EDinosaurBehaviorState::Grazing:
    case EDinosaurBehaviorState::Drinking:
    case EDinosaurBehaviorState::Sleeping:
    case EDinosaurBehaviorState::Socializing:
        return Profile.BehaviorDatabase;
    
    case EDinosaurBehaviorState::Hunting:
    case EDinosaurBehaviorState::Alert:
    case EDinosaurBehaviorState::Aggressive:
    case EDinosaurBehaviorState::Fleeing:
    case EDinosaurBehaviorState::Territorial:
        return Profile.LocomotionDatabase;
    
    default:
        return Profile.LocomotionDatabase;
    }
}