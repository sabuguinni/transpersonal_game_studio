#include "AnimationSystemManager.h"
#include "Engine/DataTable.h"
#include "Kismet/KismetMathLibrary.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"

AAnimationSystemManager::AAnimationSystemManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Configurar como singleton
    SetActorTickEnabled(false);
}

void AAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Inicializar sistema de animação
    UE_LOG(LogTemp, Warning, TEXT("Animation System Manager initialized"));
    
    // Carregar perfis padrão se disponíveis
    if (CreatureAnimationProfiles)
    {
        // Processar tabela de dados e popular cache
        TArray<FName> RowNames = CreatureAnimationProfiles->GetRowNames();
        for (const FName& RowName : RowNames)
        {
            if (FCreatureAnimationProfile* Profile = CreatureAnimationProfiles->FindRow<FCreatureAnimationProfile>(RowName, TEXT("Loading creature profiles")))
            {
                ActiveCreatureProfiles.Add(Profile->SpeciesName, *Profile);
                UE_LOG(LogTemp, Log, TEXT("Loaded animation profile for species: %s"), *Profile->SpeciesName);
            }
        }
    }
}

FCreatureAnimationProfile AAnimationSystemManager::GetCreatureProfile(const FString& SpeciesName) const
{
    if (const FCreatureAnimationProfile* Profile = ActiveCreatureProfiles.Find(SpeciesName))
    {
        return *Profile;
    }
    
    // Retornar perfil padrão se não encontrado
    FCreatureAnimationProfile DefaultProfile;
    DefaultProfile.SpeciesName = SpeciesName;
    UE_LOG(LogTemp, Warning, TEXT("No animation profile found for species: %s. Using default."), *SpeciesName);
    
    return DefaultProfile;
}

void AAnimationSystemManager::RegisterCreature(const FString& SpeciesName, const FCreatureAnimationProfile& Profile)
{
    ActiveCreatureProfiles.Add(SpeciesName, Profile);
    UE_LOG(LogTemp, Log, TEXT("Registered new creature animation profile: %s"), *SpeciesName);
}

void AAnimationSystemManager::UpdateCreatureEmotionalState(const FString& CreatureID, EEmotionalState NewState)
{
    // Esta função será chamada pelos NPCs quando mudarem de estado emocional
    // Permitirá transições suaves entre diferentes Motion Matching databases
    UE_LOG(LogTemp, Log, TEXT("Creature %s emotional state updated to: %d"), *CreatureID, (int32)NewState);
}

FCreatureAnimationProfile AAnimationSystemManager::GenerateUniqueVariation(const FCreatureAnimationProfile& BaseProfile, int32 Seed) const
{
    FCreatureAnimationProfile VariedProfile = BaseProfile;
    
    // Usar seed para gerar variações consistentes
    FRandomStream RandomStream(Seed);
    
    // Variações sutis na velocidade de caminhada (±20%)
    VariedProfile.WalkSpeedVariation = BaseProfile.WalkSpeedVariation * RandomStream.FRandRange(0.8f, 1.2f);
    
    // Variações na postura (±15%)
    VariedProfile.PostureVariation = BaseProfile.PostureVariation * RandomStream.FRandRange(0.85f, 1.15f);
    
    // Variações no padrão de caminhada (±10%)
    VariedProfile.GaitVariation = BaseProfile.GaitVariation * RandomStream.FRandRange(0.9f, 1.1f);
    
    // Pequenas variações no range do IK dos pés
    VariedProfile.FootIKRange = BaseProfile.FootIKRange * RandomStream.FRandRange(0.9f, 1.1f);
    
    UE_LOG(LogTemp, Log, TEXT("Generated unique variation for %s with seed %d"), *BaseProfile.SpeciesName, Seed);
    
    return VariedProfile;
}