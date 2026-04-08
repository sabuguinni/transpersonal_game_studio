#include "CharacterSystem.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"
#include "Components/SkeletalMeshComponent.h"

int32 UCharacterGenerationSystem::GenerationSeed = 12345;

FCharacterGeneticVariation UCharacterGenerationSystem::GenerateUniqueVariation(const UCharacterArchetype* Archetype)
{
    if (!Archetype)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterGenerationSystem: Null archetype provided"));
        return FCharacterGeneticVariation();
    }

    FCharacterGeneticVariation Variation;
    
    // Incrementa seed para garantir unicidade
    GenerationSeed++;
    FRandomStream RandomStream(GenerationSeed);

    // Gera variações faciais dentro dos limites do arquétipo
    Variation.FaceWidth = RandomStream.FRandRange(Archetype->MinVariation.FaceWidth, Archetype->MaxVariation.FaceWidth);
    Variation.EyeSize = RandomStream.FRandRange(Archetype->MinVariation.EyeSize, Archetype->MaxVariation.EyeSize);
    Variation.NoseSize = RandomStream.FRandRange(Archetype->MinVariation.NoseSize, Archetype->MaxVariation.NoseSize);
    Variation.MouthWidth = RandomStream.FRandRange(Archetype->MinVariation.MouthWidth, Archetype->MaxVariation.MouthWidth);
    Variation.JawWidth = RandomStream.FRandRange(Archetype->MinVariation.JawWidth, Archetype->MaxVariation.JawWidth);

    // Gera variações corporais
    Variation.Height = RandomStream.FRandRange(Archetype->MinVariation.Height, Archetype->MaxVariation.Height);
    Variation.BodyWeight = RandomStream.FRandRange(Archetype->MinVariation.BodyWeight, Archetype->MaxVariation.BodyWeight);
    Variation.MuscleDefinition = RandomStream.FRandRange(Archetype->MinVariation.MuscleDefinition, Archetype->MaxVariation.MuscleDefinition);

    // Gera cores interpolando entre min e max
    float SkinBlend = RandomStream.FRand();
    Variation.SkinTone = FLinearColor::LerpUsingHSV(Archetype->MinVariation.SkinTone, Archetype->MaxVariation.SkinTone, SkinBlend);

    float EyeBlend = RandomStream.FRand();
    Variation.EyeColor = FLinearColor::LerpUsingHSV(Archetype->MinVariation.EyeColor, Archetype->MaxVariation.EyeColor, EyeBlend);

    float HairBlend = RandomStream.FRand();
    Variation.HairColor = FLinearColor::LerpUsingHSV(Archetype->MinVariation.HairColor, Archetype->MaxVariation.HairColor, HairBlend);

    // Gera marcas e desgaste
    Variation.ScarIntensity = RandomStream.FRandRange(Archetype->MinVariation.ScarIntensity, Archetype->MaxVariation.ScarIntensity);
    Variation.WeatheringLevel = RandomStream.FRandRange(Archetype->MinVariation.WeatheringLevel, Archetype->MaxVariation.WeatheringLevel);
    Variation.DirtLevel = RandomStream.FRandRange(Archetype->MinVariation.DirtLevel, Archetype->MaxVariation.DirtLevel);

    UE_LOG(LogTemp, Log, TEXT("Generated unique character variation with seed %d"), GenerationSeed);

    return Variation;
}

void UCharacterGenerationSystem::ApplyGeneticVariation(USkeletalMeshComponent* MeshComponent, const FCharacterGeneticVariation& Variation)
{
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterGenerationSystem: Null mesh component provided"));
        return;
    }

    // Aplica escala corporal
    FVector CurrentScale = MeshComponent->GetComponentScale();
    float HeightScale = FMath::Lerp(0.9f, 1.1f, Variation.Height);
    float WidthScale = FMath::Lerp(0.95f, 1.05f, Variation.BodyWeight);
    
    FVector NewScale = FVector(WidthScale, WidthScale, HeightScale);
    MeshComponent->SetWorldScale3D(CurrentScale * NewScale);

    // Aplica parâmetros de material para variações de cor
    UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(0);
    if (DynamicMaterial)
    {
        DynamicMaterial->SetVectorParameterValue(FName("SkinTone"), Variation.SkinTone);
        DynamicMaterial->SetVectorParameterValue(FName("EyeColor"), Variation.EyeColor);
        DynamicMaterial->SetVectorParameterValue(FName("HairColor"), Variation.HairColor);
        DynamicMaterial->SetScalarParameterValue(FName("ScarIntensity"), Variation.ScarIntensity);
        DynamicMaterial->SetScalarParameterValue(FName("WeatheringLevel"), Variation.WeatheringLevel);
        DynamicMaterial->SetScalarParameterValue(FName("DirtLevel"), Variation.DirtLevel);

        UE_LOG(LogTemp, Log, TEXT("Applied genetic variation to character material"));
    }

    // Nota: Variações faciais específicas requerem integração com MetaHuman Creator
    // Isso será implementado através de parâmetros de morph targets quando disponível
}

FCharacterClothing UCharacterGenerationSystem::GetClothingForBiome(const UCharacterArchetype* Archetype, ECharacterBiome Biome)
{
    if (!Archetype)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterGenerationSystem: Null archetype provided for clothing selection"));
        return FCharacterClothing();
    }

    // Procura roupa específica para o bioma
    const FCharacterClothing* FoundClothing = Archetype->ClothingByBiome.Find(Biome);
    if (FoundClothing)
    {
        UE_LOG(LogTemp, Log, TEXT("Found specific clothing for biome %d"), (int32)Biome);
        return *FoundClothing;
    }

    // Fallback para roupa de floresta se não encontrar específica
    const FCharacterClothing* ForestClothing = Archetype->ClothingByBiome.Find(ECharacterBiome::Forest);
    if (ForestClothing)
    {
        UE_LOG(LogTemp, Log, TEXT("Using forest clothing as fallback for biome %d"), (int32)Biome);
        return *ForestClothing;
    }

    // Retorna roupa vazia se nada for encontrado
    UE_LOG(LogTemp, Warning, TEXT("No clothing found for archetype %s in biome %d"), *Archetype->ArchetypeName, (int32)Biome);
    return FCharacterClothing();
}

FText UCharacterGenerationSystem::GenerateBackstory(const UCharacterArchetype* Archetype)
{
    if (!Archetype || Archetype->BackstoryOptions.Num() == 0)
    {
        return FText::FromString(TEXT("Um sobrevivente misterioso com passado desconhecido."));
    }

    // Usa seed atual para selecionar história consistente
    FRandomStream RandomStream(GenerationSeed);
    int32 StoryIndex = RandomStream.RandRange(0, Archetype->BackstoryOptions.Num() - 1);

    return Archetype->BackstoryOptions[StoryIndex];
}