#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "CharacterArchetypeManager.generated.h"

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    // Protagonista
    Protagonist         UMETA(DisplayName = "Protagonista - Paleontologista"),
    
    // Sobreviventes Tribais
    TribalElder         UMETA(DisplayName = "Ancião Tribal"),
    TribalHunter        UMETA(DisplayName = "Caçador Tribal"),
    TribalGatherer      UMETA(DisplayName = "Coletor Tribal"),
    TribalShaman        UMETA(DisplayName = "Xamã Tribal"),
    TribalChild         UMETA(DisplayName = "Criança Tribal"),
    
    // Sobreviventes Isolados
    LoneWanderer        UMETA(DisplayName = "Andarilho Solitário"),
    CaveHermit          UMETA(DisplayName = "Eremita das Cavernas"),
    RiverNomad          UMETA(DisplayName = "Nómada do Rio"),
    
    // NPCs Hostis
    CannibalRaider      UMETA(DisplayName = "Invasor Canibal"),
    MadSurvivor         UMETA(DisplayName = "Sobrevivente Enlouquecido"),
    
    // NPCs Neutros/Comerciantes
    TravelingTrader     UMETA(DisplayName = "Comerciante Viajante"),
    ToolMaker           UMETA(DisplayName = "Fabricante de Ferramentas"),
    
    MAX                 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECharacterBodyType : uint8
{
    Masculine_Lean      UMETA(DisplayName = "Masculino Magro"),
    Masculine_Average   UMETA(DisplayName = "Masculino Médio"),
    Masculine_Heavy     UMETA(DisplayName = "Masculino Robusto"),
    Feminine_Lean       UMETA(DisplayName = "Feminino Magro"),
    Feminine_Average    UMETA(DisplayName = "Feminino Médio"),
    Feminine_Heavy      UMETA(DisplayName = "Feminino Robusto"),
    Child_Small         UMETA(DisplayName = "Criança Pequena"),
    Child_Medium        UMETA(DisplayName = "Criança Média"),
    Elder_Frail         UMETA(DisplayName = "Idoso Frágil")
};

UENUM(BlueprintType)
enum class ECharacterSkinTone : uint8
{
    VeryLight           UMETA(DisplayName = "Muito Claro"),
    Light               UMETA(DisplayName = "Claro"),
    Medium              UMETA(DisplayName = "Médio"),
    Tan                 UMETA(DisplayName = "Bronzeado"),
    Dark                UMETA(DisplayName = "Escuro"),
    VeryDark            UMETA(DisplayName = "Muito Escuro")
};

UENUM(BlueprintType)
enum class ECharacterClothing : uint8
{
    // Roupas Primitivas
    AnimalHides         UMETA(DisplayName = "Peles de Animal"),
    WovenFibers         UMETA(DisplayName = "Fibras Tecidas"),
    BarkCloth           UMETA(DisplayName = "Tecido de Casca"),
    LeatherWraps        UMETA(DisplayName = "Envoltórios de Couro"),
    
    // Roupas Específicas
    ShamanRobes         UMETA(DisplayName = "Vestes de Xamã"),
    HunterGear          UMETA(DisplayName = "Equipamento de Caçador"),
    GathererPouch       UMETA(DisplayName = "Bolsa de Coletor"),
    
    // Estados de Desgaste
    Tattered            UMETA(DisplayName = "Esfarrapado"),
    Patched             UMETA(DisplayName = "Remendado"),
    Pristine            UMETA(DisplayName = "Pristino")
};

USTRUCT(BlueprintType)
struct FCharacterVariationData
{
    GENERATED_BODY()

    // Identificação
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CharacterName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterArchetype Archetype;
    
    // Aparência Física
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterBodyType BodyType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterSkinTone SkinTone;
    
    // MetaHuman Blend Shapes (0.0 - 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FaceWidth;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EyeSize;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NoseSize;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LipThickness;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CheekboneHeight;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float JawWidth;
    
    // Cabelo
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString HairAssetPath;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor HairColor;
    
    // Roupa e Acessórios
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterClothing ClothingType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> AccessoryPaths;
    
    // Marcas de Vida (cicatrizes, tatuagens, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> LifeMarkTextures;
    
    // Postura e Animação
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString IdleAnimationSet;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PostureVariation; // -1.0 (curvado) to 1.0 (ereto)

    FCharacterVariationData()
    {
        CharacterName = TEXT("Unnamed");
        Archetype = ECharacterArchetype::TribalGatherer;
        BodyType = ECharacterBodyType::Masculine_Average;
        SkinTone = ECharacterSkinTone::Medium;
        FaceWidth = 0.5f;
        EyeSize = 0.5f;
        NoseSize = 0.5f;
        LipThickness = 0.5f;
        CheekboneHeight = 0.5f;
        JawWidth = 0.5f;
        HairColor = FLinearColor::Black;
        ClothingType = ECharacterClothing::AnimalHides;
        PostureVariation = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterArchetypeManager : public UDataAsset
{
    GENERATED_BODY()

public:
    // Database de variações por arquétipo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Database")
    TMap<ECharacterArchetype, TArray<FCharacterVariationData>> ArchetypeVariations;
    
    // Configurações de geração procedural
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation")
    int32 MaxVariationsPerArchetype = 50;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation")
    bool bEnableProceduralGeneration = true;
    
    // Métodos de geração
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FCharacterVariationData GenerateRandomVariation(ECharacterArchetype Archetype);
    
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FCharacterVariationData GetSpecificVariation(ECharacterArchetype Archetype, int32 VariationIndex);
    
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void PopulateArchetypeDatabase();
    
private:
    // Geração procedural de características faciais
    float GenerateBlendShapeValue(ECharacterArchetype Archetype, const FString& BlendShapeName);
    
    // Seleção de assets baseada no arquétipo
    FString SelectHairAsset(ECharacterArchetype Archetype, ECharacterBodyType BodyType);
    ECharacterClothing SelectClothing(ECharacterArchetype Archetype);
    TArray<FString> SelectAccessories(ECharacterArchetype Archetype);
};