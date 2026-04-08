#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/DataAsset.h"
#include "CharacterSystem.generated.h"

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    // PROTAGONISTA
    Paleontologist          UMETA(DisplayName = "Paleontologista"),
    
    // TRIBOS HUMANAS PRÉ-HISTÓRICAS
    TribalElder            UMETA(DisplayName = "Ancião Tribal"),
    TribalHunter           UMETA(DisplayName = "Caçador Tribal"),
    TribalShaman           UMETA(DisplayName = "Xamã Tribal"),
    TribalCrafter          UMETA(DisplayName = "Artesão Tribal"),
    TribalChild            UMETA(DisplayName = "Criança Tribal"),
    
    // SOBREVIVENTES PERDIDOS
    LostExplorer           UMETA(DisplayName = "Explorador Perdido"),
    InjuredSurvivor        UMETA(DisplayName = "Sobrevivente Ferido"),
    WildHermit             UMETA(DisplayName = "Eremita Selvagem"),
    
    // NPCS ESPECIAIS
    MysteriousStranger     UMETA(DisplayName = "Estranho Misterioso"),
    AncientGuardian        UMETA(DisplayName = "Guardião Antigo"),
    
    MAX                    UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECharacterGender : uint8
{
    Male                   UMETA(DisplayName = "Masculino"),
    Female                 UMETA(DisplayName = "Feminino"),
    NonBinary              UMETA(DisplayName = "Não-Binário")
};

UENUM(BlueprintType)
enum class ECharacterAge : uint8
{
    Child                  UMETA(DisplayName = "Criança (8-12)"),
    Teenager               UMETA(DisplayName = "Adolescente (13-17)"),
    YoungAdult             UMETA(DisplayName = "Jovem Adulto (18-25)"),
    Adult                  UMETA(DisplayName = "Adulto (26-45)"),
    MiddleAged             UMETA(DisplayName = "Meia-Idade (46-60)"),
    Elder                  UMETA(DisplayName = "Idoso (61+)")
};

UENUM(BlueprintType)
enum class ECharacterEthnicity : uint8
{
    African                UMETA(DisplayName = "Africano"),
    Asian                  UMETA(DisplayName = "Asiático"),
    European               UMETA(DisplayName = "Europeu"),
    Indigenous             UMETA(DisplayName = "Indígena"),
    MiddleEastern          UMETA(DisplayName = "Oriente Médio"),
    Mixed                  UMETA(DisplayName = "Misto")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float SkinTone = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float EyeColor = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float HairColor = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Height = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Weight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Muscle = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float FaceWidth = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float JawWidth = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float NoseSize = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float EyeSize = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Scars")
    TArray<FString> BattleScars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float SkinWeathering = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float SunDamage = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterClothing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> HeadGear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> Torso;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> Arms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> Legs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> Feet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessories")
    TArray<TSoftObjectPtr<USkeletalMesh>> Accessories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    float ClothingWear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    float DirtLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    float BloodStains = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCharacterArchetypeData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString ArchetypeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demographics")
    TArray<ECharacterGender> AllowedGenders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demographics")
    TArray<ECharacterAge> AllowedAges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demographics")
    TArray<ECharacterEthnicity> AllowedEthnicities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FCharacterVariation BaseVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FCharacterClothing> ClothingOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float TrustLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float IntelligenceLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> PossibleBackstories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<USkeletalMesh> BaseMetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FString MetaHumanPresetID;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCharacterGenerator : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static FCharacterVariation GenerateRandomVariation(ECharacterArchetype Archetype, ECharacterGender Gender, ECharacterAge Age, ECharacterEthnicity Ethnicity);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static FCharacterClothing SelectClothingForArchetype(ECharacterArchetype Archetype, float WearLevel = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static TArray<FString> GenerateBattleScars(ECharacterArchetype Archetype, ECharacterAge Age);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static float CalculateWeatheringLevel(ECharacterArchetype Archetype, ECharacterAge Age);

private:
    static TMap<ECharacterArchetype, UCharacterArchetypeData*> ArchetypeDatabase;
    static void LoadArchetypeDatabase();
};