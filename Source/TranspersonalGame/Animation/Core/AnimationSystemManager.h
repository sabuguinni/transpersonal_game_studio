#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Animation/AnimInstance.h"
#include "AnimationSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECreatureType : uint8
{
    Player,
    SmallHerbivore,
    LargeHerbivore,
    SmallCarnivore,
    LargeCarnivore,
    Apex
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Neutral,
    Curious,
    Afraid,
    Aggressive,
    Trusting,
    Protective,
    Hunting,
    Feeding,
    Resting
};

USTRUCT(BlueprintType)
struct FCreatureAnimationProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECreatureType CreatureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    // Motion Matching Database para cada estado emocional
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EEmotionalState, class UPoseSearchDatabase*> EmotionalStateDatabases;

    // Variações físicas únicas
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WalkSpeedVariation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PostureVariation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GaitVariation = 1.0f;

    // IK Settings para adaptação ao terreno
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FootIKRange = 50.0f;

    FCreatureAnimationProfile()
    {
        CreatureType = ECreatureType::SmallHerbivore;
        SpeciesName = TEXT("Unknown");
        WalkSpeedVariation = 1.0f;
        PostureVariation = 1.0f;
        GaitVariation = 1.0f;
        bUseFootIK = true;
        FootIKRange = 50.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAnimationSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAnimationSystemManager();

protected:
    virtual void BeginPlay() override;

    // Database central de perfis de animação
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation System")
    UDataTable* CreatureAnimationProfiles;

    // Motion Matching Schema para diferentes tipos de criaturas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<ECreatureType, class UPoseSearchSchema*> MotionMatchingSchemas;

    // Configurações globais de IK
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    bool bGlobalFootIKEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float GlobalIKIntensity = 1.0f;

public:
    // Função para obter perfil de animação de uma criatura
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    FCreatureAnimationProfile GetCreatureProfile(const FString& SpeciesName) const;

    // Função para registrar nova criatura no sistema
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void RegisterCreature(const FString& SpeciesName, const FCreatureAnimationProfile& Profile);

    // Função para atualizar estado emocional de uma criatura
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateCreatureEmotionalState(const FString& CreatureID, EEmotionalState NewState);

    // Sistema de variação procedimental
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    FCreatureAnimationProfile GenerateUniqueVariation(const FCreatureAnimationProfile& BaseProfile, int32 Seed) const;

private:
    // Cache de perfis ativos
    TMap<FString, FCreatureAnimationProfile> ActiveCreatureProfiles;

    // Sistema de tracking de criaturas únicas
    TMap<FString, int32> CreatureUniqueSeeds;
};